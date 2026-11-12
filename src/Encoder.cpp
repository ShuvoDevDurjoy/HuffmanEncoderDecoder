#include "../includes/Encoder.hpp"

#include <algorithm> // For std::max
#include <fstream>
#include <limits>
#include <system_error>

namespace
{
    struct DirectoryEntry
    {
        bool is_directory;
        bool is_symlink;
        std::string relative_path;
        std::string symlink_target;
        uint64_t size;
    };

    bool write_u8(ByteWriter *writer, uint8_t value)
    {
        return writer->write(value);
    }

    bool write_be(ByteWriter *writer, uint64_t value, size_t count)
    {
        for (size_t index = count; index > 0; --index)
        {
            uint8_t byte = static_cast<uint8_t>(value >> ((index - 1) * 8));
            if (!writer->write(byte))
                return false;
        }
        return true;
    }

    bool write_string16(ByteWriter *writer, const std::string &value)
    {
        return value.size() <= UINT16_MAX && write_be(writer, value.size(), 2) &&
               (value.empty() || writer->write_buffer(reinterpret_cast<const uint8_t *>(value.data()), value.size()));
    }
}

uint8_t Encoder::_get_padding(const std::array<TYPE_FREQUENCY, BYTE_SIZE> &freq,
                              const std::array<ByteStream, BYTE_SIZE> &codes)
{
    size_t bits_mod_8 = 0;
    for (size_t symbol = 0; symbol < BYTE_SIZE; ++symbol)
    {
        if (freq[symbol] == 0)
            continue;

        const size_t code_bits = codes[symbol].size() * 8 - codes[symbol].get_pad();
        bits_mod_8 = (bits_mod_8 + (freq[symbol] % 8) * (code_bits % 8)) % 8;
    }
    return static_cast<uint8_t>((8 - bits_mod_8) % 8);
}

bool Encoder::_encode_content(std::array<ByteStream, BYTE_SIZE> &codes, size_t total_bytes)
{
    constexpr size_t OUTPUT_BUFFER_SIZE = 64 * 1024;
    std::array<uint8_t, OUTPUT_BUFFER_SIZE> output_buffer{};
    size_t output_size = 0;
    size_t step_size = std::max<size_t>(1, total_bytes / 100);
    size_t read_bytes = 0;
    uint8_t last_progress = 0;
    uint8_t output_byte = 0;
    uint8_t used_bits = 0;

    Terminal::run_with_progress("Encoding Content", 0);

    uint8_t byte;
    while (in_file->next_byte(byte))
    {
        if (codes[byte].size() == 0)
            return false;
        const size_t code_bits = codes[byte].size() * 8 - codes[byte].get_pad();
        for (size_t i = 0; i < code_bits; ++i)
        {
            bool bit = false;
            if (!codes[byte].get_bit(i, bit))
                return false;
            output_byte = static_cast<uint8_t>((output_byte << 1) | bit);
            if (++used_bits == 8)
            {
                output_buffer[output_size++] = output_byte;
                output_byte = 0;
                used_bits = 0;
                if (output_size == output_buffer.size())
                {
                    if (!out_file->write_buffer(output_buffer.data(), output_size))
                        return false;
                    output_size = 0;
                }
            }
        }
        read_bytes++;

        // if a percent is read then trigger progress update
        if (read_bytes % step_size == 0)
        {
            uint8_t current_progress = static_cast<uint8_t>((read_bytes * 100) / total_bytes);

            // Only update terminal UI if percentage actually changed
            if (current_progress > last_progress)
            {
                Terminal::run_with_progress("Encoding Content", current_progress);
                last_progress = current_progress;
            }
        }
    }

    if (used_bits != 0)
        output_buffer[output_size++] = static_cast<uint8_t>(output_byte << (8 - used_bits));
    if (output_size != 0 && !out_file->write_buffer(output_buffer.data(), output_size))
        return false;

    // Ensure terminal shows 100% when loop finishes
    Terminal::run_with_progress("Encoding Content", 100);
    std::cout << std::endl;

    return true;
}

bool Encoder::_encode(EncodedContent& content){
    ByteStream file_name_size;
    Utils::uint16_t_to_bytestream(content.file_name_size, file_name_size);
    return out_file->write(content.magic_code_size) &&
           out_file->write(content.magic_code) &&
           out_file->write(content.version_number) &&
           out_file->write(file_name_size) &&
           out_file->write(content.file_name_bytes) &&
           out_file->write(content.num_unique_chars) &&
           out_file->write(content.max_freq_byte_size) &&
           out_file->write(content.frequency_codes) &&
           out_file->write(content.padding);
}

bool Encoder::_encode_directory(const std::filesystem::path &directory, const std::string &output_path)
{
    std::error_code error;
    const std::filesystem::path root = std::filesystem::absolute(directory, error).lexically_normal();
    if (error)
        return false;

    const std::string root_name = root.filename().string();
    if (root_name.empty() || root_name == "." || root_name == ".." ||
        root_name.find_first_of("/\\:") != std::string::npos || root_name.size() > UINT16_MAX)
        return false;

    const std::filesystem::path absolute_output = std::filesystem::absolute(output_path, error).lexically_normal();
    if (error)
        return false;
    const std::filesystem::path relative_output = absolute_output.lexically_relative(root);
    if (!relative_output.empty() && relative_output != "." && *relative_output.begin() != "..")
        return false;

    std::vector<DirectoryEntry> entries;
    size_t path_bytes = 0;
    constexpr size_t MAX_ENTRIES = 100000;
    constexpr size_t MAX_PATH_BYTES = 16 * 1024 * 1024;
    Terminal::stat_carriage("Scanning Folder Entries: ", Status::Modern::PROGRESS);
    std::filesystem::recursive_directory_iterator iterator(root, error), end;
    if (error)
        return false;
    for (; iterator != end; iterator.increment(error))
    {
        if (error)
            return false;
        if (entries.size() != 0 && entries.size() % 1000 == 0)
            Terminal::stat_carriage("Scanning Folder Entries (" + std::to_string(entries.size()) + "): ", Status::Modern::PROGRESS);
        const auto status = iterator->symlink_status(error);
        if (error)
            return false;

        DirectoryEntry entry{};
        entry.relative_path = iterator->path().lexically_relative(root).generic_string();
        if (entry.relative_path.empty() || entry.relative_path.size() > UINT16_MAX ||
            entry.relative_path.find_first_of("\\:") != std::string::npos ||
            std::filesystem::path(entry.relative_path).lexically_normal().generic_string() != entry.relative_path)
            return false;
        if (entries.size() >= MAX_ENTRIES || entry.relative_path.size() > MAX_PATH_BYTES - path_bytes)
            return false;
        path_bytes += entry.relative_path.size();

        if (std::filesystem::is_symlink(status))
        {
            const std::filesystem::path target = std::filesystem::read_symlink(iterator->path(), error);
            if (error || target.empty() || target.is_absolute() || target.has_root_name())
                return false;
            const std::filesystem::path resolved =
                (std::filesystem::path(entry.relative_path).parent_path() / target).lexically_normal();
            if (resolved.empty() || *resolved.begin() == "..")
                return false;
            entry.is_symlink = true;
            entry.symlink_target = target.generic_string();
            if (entry.symlink_target.size() > UINT16_MAX ||
                entry.symlink_target.find_first_of("\\:") != std::string::npos)
                return false;
            if (entry.symlink_target.size() > MAX_PATH_BYTES - path_bytes)
                return false;
            path_bytes += entry.symlink_target.size();
        }
        else if (std::filesystem::is_directory(status))
        {
            entry.is_directory = true;
            entry.size = 0;
        }
        else if (std::filesystem::is_regular_file(status))
        {
            entry.is_directory = false;
            entry.size = std::filesystem::file_size(root / entry.relative_path, error);
            if (error)
                return false;
        }
        else
        {
            return false;
        }
        entries.push_back(std::move(entry));
    }
    if (error)
        return false;
    Terminal::stat_carriage("Scanning Folder Entries (" + std::to_string(entries.size()) + "): ", Status::Modern::SUCCESS);
    std::cout << std::endl;

    std::sort(entries.begin(), entries.end(), [](const DirectoryEntry &left, const DirectoryEntry &right)
              { return left.relative_path < right.relative_path; });

    std::array<TYPE_FREQUENCY, BYTE_SIZE> freq{};
    uint64_t total_bytes = 0;
    for (const DirectoryEntry &entry : entries)
    {
        if (entry.is_directory || entry.is_symlink)
            continue;
        if (entry.size > UINT64_MAX - total_bytes)
            return false;
        total_bytes += entry.size;
    }

    uint64_t scanned_bytes = 0;
    uint8_t scan_progress = 0;
    Terminal::run_with_progress("Scanning Folder Data", 0);
    for (const DirectoryEntry &entry : entries)
    {
        if (entry.is_directory || entry.is_symlink)
            continue;

        std::ifstream input(root / entry.relative_path, std::ios::binary);
        if (!input)
            return false;
        std::array<uint8_t, 64 * 1024> input_buffer{};
        uint64_t bytes_read = 0;
        while (input)
        {
            input.read(reinterpret_cast<char *>(input_buffer.data()), static_cast<std::streamsize>(input_buffer.size()));
            const std::streamsize count = input.gcount();
            for (std::streamsize index = 0; index < count; ++index)
            {
                const uint8_t symbol = input_buffer[static_cast<size_t>(index)];
                if (freq[symbol] == UINT64_MAX)
                    return false;
                ++freq[symbol];
            }
            bytes_read += static_cast<uint64_t>(count);
            scanned_bytes += static_cast<uint64_t>(count);
            const uint8_t current_progress = total_bytes == 0
                                                 ? 100
                                                 : static_cast<uint8_t>((static_cast<long double>(scanned_bytes) * 100.0L) / total_bytes);
            if (current_progress > scan_progress)
            {
                Terminal::run_with_progress("Scanning Folder Data", current_progress);
                scan_progress = current_progress;
            }
        }
        if (!input.eof() || bytes_read != entry.size)
            return false;
    }
    if (scan_progress < 100)
        Terminal::run_with_progress("Scanning Folder Data", 100);
    std::cout << std::endl;

    uint16_t unique_symbols = 0;
    uint64_t max_frequency = 0;
    for (TYPE_FREQUENCY frequency : freq)
    {
        if (frequency != 0)
            ++unique_symbols;
        max_frequency = std::max(max_frequency, frequency);
    }
    const uint8_t frequency_bytes = Utils::min_byte_count(max_frequency);
    Node *root_node = Utils::build_huffman_tree(freq);
    std::array<ByteStream, BYTE_SIZE> huffman_codes;
    if (root_node)
    {
        ByteStream code;
        Utils::build_huffman_codes(root_node, code, huffman_codes);
    }

    size_t bits_mod_8 = 0;
    for (size_t symbol = 0; symbol < BYTE_SIZE; ++symbol)
    {
        if (freq[symbol] == 0)
            continue;
        const size_t code_bits = huffman_codes[symbol].size() * 8 - huffman_codes[symbol].get_pad();
        bits_mod_8 = (bits_mod_8 + (freq[symbol] % 8) * (code_bits % 8)) % 8;
    }
    const uint8_t padding = static_cast<uint8_t>((8 - bits_mod_8) % 8);

    if (!out_file->open(output_path))
    {
        delete root_node;
        return false;
    }

    const std::string magic = MAGIC_CODE;
    bool success = write_u8(out_file, static_cast<uint8_t>(magic.size())) &&
                   out_file->write_buffer(reinterpret_cast<const uint8_t *>(magic.data()), magic.size()) &&
                   write_u8(out_file, 2) && write_string16(out_file, root_name) &&
                   write_be(out_file, static_cast<uint32_t>(entries.size()), 4);
    for (const DirectoryEntry &entry : entries)
    {
        const uint8_t type = entry.is_directory ? 0 : entry.is_symlink ? 2 : 1;
        success = success && write_u8(out_file, type) && write_string16(out_file, entry.relative_path) &&
                  write_be(out_file, entry.size, 8);
        if (success && entry.is_symlink)
            success = write_string16(out_file, entry.symlink_target);
    }
    success = success && write_be(out_file, unique_symbols, 2) && write_u8(out_file, frequency_bytes);
    for (size_t symbol = 0; success && symbol < BYTE_SIZE; ++symbol)
    {
        if (freq[symbol] == 0)
            continue;
        success = write_u8(out_file, static_cast<uint8_t>(symbol)) && write_be(out_file, freq[symbol], frequency_bytes);
    }
    success = success && write_u8(out_file, padding);

    constexpr size_t OUTPUT_BUFFER_SIZE = 64 * 1024;
    std::array<uint8_t, OUTPUT_BUFFER_SIZE> output_buffer{};
    size_t output_size = 0;
    uint8_t output_byte = 0;
    uint8_t used_bits = 0;
    uint64_t encoded_input_bytes = 0;
    uint8_t encode_progress = 0;
    Terminal::run_with_progress("Encoding Folder", 0);
    for (const DirectoryEntry &entry : entries)
    {
        if (!success || entry.is_directory || entry.is_symlink)
            continue;
        std::ifstream input(root / entry.relative_path, std::ios::binary);
        if (!input)
        {
            success = false;
            break;
        }
        std::array<uint8_t, OUTPUT_BUFFER_SIZE> input_buffer{};
        uint64_t file_bytes = 0;
        uint64_t reported_file_bytes = 0;
        while (input && success)
        {
            input.read(reinterpret_cast<char *>(input_buffer.data()), static_cast<std::streamsize>(input_buffer.size()));
            const std::streamsize count = input.gcount();
            for (std::streamsize index = 0; index < count && success; ++index)
            {
                const uint8_t symbol = input_buffer[static_cast<size_t>(index)];
                if (freq[symbol] == 0)
                {
                    success = false;
                    break;
                }
                const size_t code_bits = huffman_codes[symbol].size() * 8 - huffman_codes[symbol].get_pad();
                for (size_t bit_index = 0; bit_index < code_bits; ++bit_index)
                {
                    bool bit = false;
                    if (!huffman_codes[symbol].get_bit(bit_index, bit))
                    {
                        success = false;
                        break;
                    }
                    output_byte = static_cast<uint8_t>((output_byte << 1) | bit);
                    if (++used_bits == 8)
                    {
                        output_buffer[output_size++] = output_byte;
                        output_byte = 0;
                        used_bits = 0;
                        if (output_size == output_buffer.size())
                        {
                            success = out_file->write_buffer(output_buffer.data(), output_size);
                            output_size = 0;
                        }
                    }
                }
                ++file_bytes;
            }
            encoded_input_bytes += file_bytes - reported_file_bytes;
            reported_file_bytes = file_bytes;
            const uint8_t current_progress = total_bytes == 0
                                                 ? 100
                                                 : static_cast<uint8_t>((static_cast<long double>(encoded_input_bytes) * 100.0L) / total_bytes);
            if (current_progress > encode_progress)
            {
                Terminal::run_with_progress("Encoding Folder", current_progress);
                encode_progress = current_progress;
            }
        }
        if (!input.eof() || file_bytes != entry.size)
            success = false;
    }
    if (success && used_bits != 0)
        output_buffer[output_size++] = static_cast<uint8_t>(output_byte << (8 - used_bits));
    if (success && output_size != 0)
        success = out_file->write_buffer(output_buffer.data(), output_size);
    success = success && encoded_input_bytes == total_bytes;
    if (success && encode_progress < 100)
        Terminal::run_with_progress("Encoding Folder", 100);
    std::cout << std::endl;

    out_file->close();
    delete root_node;
    return success;
}

bool Encoder::encode(std::string in_file, std::string out_file)
{
    std::error_code error;
    if (std::filesystem::is_directory(in_file, error))
        return _encode_directory(in_file, out_file);
    if(!Utils::file_exists(in_file))
        return false;
    this->in_file->open(in_file);
    this->out_file->open(out_file);

    if(!(this->in_file->is_open() && this->out_file->is_open())){
        this->in_file->close();
        this->out_file->close();
        return false;
    }

    EncodedContent content;
    content.version_number = 1;

    std::filesystem::path file_path;
    bool success;
    success = this->in_file->get_file_path(file_path);
    if(!success)
        return false;

    Utils::convert_string_to_stream(Utils::get_truncated_file_name_from_path(file_path), content.file_name_bytes);
    content.file_name_size = content.file_name_bytes.size();

    Utils::convert_string_to_stream(this->encoder_magic_code, content.magic_code);
    content.magic_code_size = static_cast<uint8_t>(content.magic_code.size());

    std::array<uint64_t, BYTE_SIZE> freq;
    uint16_t num_unique_chars;
    Utils::build_frequency_map(freq, num_unique_chars, content.max_freq_byte_size, this->in_file);
    Utils::uint16_t_to_bytestream(num_unique_chars, content.num_unique_chars);
    Utils::frequency_to_stream(freq, content.frequency_codes, content.max_freq_byte_size);

    this->in_file->reset();
    Node* root = Utils::build_huffman_tree(freq);
    if(!root){
        this->in_file->close();
        this->out_file->close();
        return false;
    }

    std::array<ByteStream, BYTE_SIZE> huffman_codes;
    ByteStream code;
    Utils::build_huffman_codes(root, code, huffman_codes);
    content.padding = _get_padding(freq, huffman_codes);
    success = _encode(content);
    if (success)
        success = this->_encode_content(huffman_codes, this->in_file->size());

    delete root;

    this->in_file->close();
    this->out_file->close();
    return success;
}
