#include "../includes/Decoder.hpp"

#include <algorithm>
#include <fstream>
#include <set>
#include <system_error>

namespace
{
    bool read_be(FileReader *reader, uint64_t &value, size_t count)
    {
        value = 0;
        uint8_t byte = 0;
        for (size_t index = 0; index < count; ++index)
        {
            if (!reader->next_byte(byte))
                return false;
            value = (value << 8) | byte;
        }
        return true;
    }

    bool safe_root_name(const std::string &name)
    {
        return !name.empty() && name != "." && name != ".." &&
               name.find_first_of("/\\:") == std::string::npos;
    }

    bool safe_relative_path(const std::string &name)
    {
        if (name.empty() || name.find_first_of("\\:") != std::string::npos)
            return false;
        const std::filesystem::path path(name);
        if (path.is_absolute() || path.has_root_name() || path.lexically_normal().generic_string() != name)
            return false;
        for (const auto &part : path)
        {
            if (part == "." || part == "..")
                return false;
        }
        return true;
    }

    bool safe_symlink_target(const std::string &link_name, const std::string &target_name)
    {
        if (target_name.empty() || target_name.find_first_of("\\:") != std::string::npos)
            return false;
        const std::filesystem::path target(target_name);
        if (target.is_absolute() || target.has_root_name())
            return false;
        const std::filesystem::path resolved =
            (std::filesystem::path(link_name).parent_path() / target).lexically_normal();
        return !resolved.empty() && *resolved.begin() != "..";
    }

    bool has_symlink_component(const std::filesystem::path &root, const std::filesystem::path &relative)
    {
        std::error_code error;
        std::filesystem::path current = root;
        auto status = std::filesystem::symlink_status(current, error);
        if (!error && std::filesystem::is_symlink(status))
            return true;
        error.clear();
        for (const auto &part : relative)
        {
            current /= part;
            status = std::filesystem::symlink_status(current, error);
            if (!error && std::filesystem::is_symlink(status))
                return true;
            error.clear();
        }
        return false;
    }
}

Decoder::Decoder()
{
    writer = new ByteWriter();
    reader = new FileReader();
}

Decoder::~Decoder()
{
    delete writer;
    delete reader;
}

bool Decoder::pretest(std::string &in_file, std::string &out_dir)
{
    if (!Utils::file_exists(in_file) || !Utils::dir_exists(out_dir))
    {
        return false;
    }

    reader->open(in_file);

    if (!(reader->is_open()))
        return on_fail();
    return true;
}

bool Decoder::read_metadata(const std::string &out_dir, std::string &out_path)
{
    std::string magic_code;
    uint8_t v_n;
    if (!Utils::read_string(reader, magic_code))
        return on_fail();
    if (magic_code != this->decoder_magic_code)
        return on_fail();
    if (!Utils::read_uint8_t(reader, v_n))
        return on_fail();
    if (v_n != this->version_number && v_n != 2)
        return on_fail();
    archive_version = v_n;

    if (archive_version == 1)
    {
        std::string out_file_name;
        if (!Utils::read_string2(reader, out_file_name))
            return on_fail();

        out_path = (std::filesystem::path(out_dir) / out_file_name).string();
        if (!writer->open(out_path))
            return on_fail();
        return true;
    }

    std::string root_name;
    uint64_t entry_count = 0;
    if (!Utils::read_string2(reader, root_name) || !safe_root_name(root_name) ||
        !read_be(reader, entry_count, 4) || entry_count > 100000)
        return on_fail();

    const std::filesystem::path output_root = std::filesystem::path(out_dir) / root_name;
    std::error_code error;
    if (has_symlink_component(std::filesystem::path(out_dir), root_name))
        return on_fail();
    std::filesystem::create_directories(output_root, error);
    if (error)
        return on_fail();

    archive_entries.clear();
    archive_entries.reserve(static_cast<size_t>(entry_count));
    archive_symlinks.clear();
    std::set<std::string> seen_paths;
    size_t path_bytes = 0;
    for (uint64_t index = 0; index < entry_count; ++index)
    {
        uint8_t type = 0;
        std::string relative_name;
        uint64_t file_size = 0;
        if (!reader->next_byte(type) || type > 2 || !Utils::read_string2(reader, relative_name) ||
            !safe_relative_path(relative_name) || !read_be(reader, file_size, 8) ||
            (type != 1 && file_size != 0) || !seen_paths.insert(relative_name).second)
            return on_fail();
        if (relative_name.size() > 16 * 1024 * 1024 - path_bytes)
            return on_fail();
        path_bytes += relative_name.size();

        const std::filesystem::path relative_path(relative_name);
        if (has_symlink_component(output_root, relative_path))
            return on_fail();
        const std::filesystem::path target = output_root / relative_path;
        if (type == 2)
        {
            std::string link_target;
            if (!Utils::read_string2(reader, link_target) || !safe_symlink_target(relative_name, link_target))
                return on_fail();
            if (link_target.size() > 16 * 1024 * 1024 - path_bytes)
                return on_fail();
            path_bytes += link_target.size();
            archive_symlinks.push_back({target.string(), std::move(link_target)});
        }
        else if (type == 0)
        {
            std::filesystem::create_directories(target, error);
            if (error)
                return on_fail();
        }
        else
        {
            std::filesystem::create_directories(target.parent_path(), error);
            if (error)
                return on_fail();
            std::ofstream output(target, std::ios::binary | std::ios::trunc);
            if (!output)
                return on_fail();
            output.close();
            archive_entries.push_back({false, target.string(), file_size});
        }
    }
    out_path = output_root.string();

    return true;
}

bool Decoder::on_fail()
{
    writer->close();
    reader->close();
    return false;
}

bool Decoder::_read_chars(std::array<TYPE_FREQUENCY, BYTE_SIZE> &freq, uint16_t uni_chars, uint8_t max_freq_byte_size)
{
    if (uni_chars > BYTE_SIZE || max_freq_byte_size == 0 || max_freq_byte_size > 8)
        return false;
    Terminal::stat_carriage("Building Frequency Table: ", Status::Modern::PROGRESS);
    size_t num_iter = static_cast<size_t>(uni_chars);
    uint8_t byte = 0;
    uint64_t f = 0;
    for (size_t i = 0; i < num_iter; ++i)
    {
        f = 0;
        if (reader->next_byte(byte) && Utils::read_uint64_t(reader, f, max_freq_byte_size))
        {
            if (freq[byte] != 0 || f == 0)
                return false;
            freq[byte] = f;
        }
        else
        {
            return false;
        }
    }
    Terminal::stat_carriage("Building Frequency Table: ", Status::Modern::SUCCESS);
    return true;
}

bool Decoder::_decode_stream(Node *root, uint64_t expected_size, uint8_t padding,
                             const std::vector<ArchiveEntry> *entries)
{
    if (padding > 7)
        return false;
    if (expected_size == 0)
        return root == nullptr && padding == 0;
    if (!root)
        return false;

    constexpr size_t OUTPUT_BUFFER_SIZE = 64 * 1024;
    std::array<uint8_t, OUTPUT_BUFFER_SIZE> output_buffer{};
    size_t output_size = 0;
    uint64_t decoded_size = 0;
    Node *current = root;
    const uint64_t progress_step = std::max<uint64_t>(1, expected_size / 100);
    uint8_t last_progress = 0;
    uint8_t encoded_byte = 0;
    size_t entry_index = 0;
    uint64_t file_remaining = 0;

    auto flush_output = [&]()
    {
        return output_size == 0 || writer->write_buffer(output_buffer.data(), output_size);
    };
    auto open_next_entry = [&]()
    {
        while (entries && entry_index < entries->size())
        {
            const ArchiveEntry &entry = (*entries)[entry_index++];
            if (entry.size == 0)
                continue;
            file_remaining = entry.size;
            return writer->open(entry.output_path);
        }
        return !entries;
    };

    if (entries && !open_next_entry())
        return false;

    Terminal::run_with_progress("Decoding Content", 0);

    while (decoded_size < expected_size && reader->next_byte(encoded_byte))
    {
        for (int bit_index = 7; bit_index >= 0 && decoded_size < expected_size; --bit_index)
        {
            if (root->is_leaf())
            {
                output_buffer[output_size++] = root->c;
            }
            else
            {
                const bool bit = ((encoded_byte >> bit_index) & 1) != 0;
                current = bit ? current->right : current->left;
                if (!current)
                    return false;
                if (current->is_leaf())
                {
                    output_buffer[output_size++] = current->c;
                    current = root;
                }
                else
                {
                    continue;
                }
            }

            ++decoded_size;
            if (entries)
            {
                if (file_remaining == 0)
                    return false;
                --file_remaining;
                if (file_remaining == 0)
                {
                    if (!flush_output())
                        return false;
                    output_size = 0;
                    writer->close();
                    if (decoded_size < expected_size && !open_next_entry())
                        return false;
                }
            }
            if (output_size == output_buffer.size())
            {
                if (!flush_output())
                    return false;
                output_size = 0;
            }
        }

        const uint64_t progress_value = decoded_size / progress_step;
        if (progress_value > last_progress)
        {
            last_progress = static_cast<uint8_t>(std::min<uint64_t>(99, progress_value));
            Terminal::run_with_progress("Decoding Content", last_progress);
        }
    }

    if (decoded_size != expected_size || (current != root && !root->is_leaf()))
        return false;
    if (entries)
    {
        while (entry_index < entries->size() && (*entries)[entry_index].size == 0)
            ++entry_index;
    }
    if (entries && entry_index != entries->size())
        return false;
    if (!flush_output())
        return false;
    if (entries)
        writer->close();

    Terminal::run_with_progress("Decoding Content", 100);
    std::cout << std::endl;
    return true;
}

bool Decoder::decode(std::string in_file, std::string &out_dir, std::string &out_path)
{
    bool success;
    if (!pretest(in_file, out_dir))
        return false;

    DecodedContent content = DecodedContent();
    std::array<TYPE_FREQUENCY, BYTE_SIZE> freq;
    freq.fill(0);

    if (!read_metadata(out_dir, out_path))
        return on_fail();
    if (!Utils::read_uint16_t(reader, content.num_unique_chars))
        return on_fail();
    if (!Utils::read_uint8_t(reader, content.max_freq_byte_size))
        return on_fail();
    if (!_read_chars(freq, content.num_unique_chars, content.max_freq_byte_size))
        return on_fail();
    if (!reader->next_byte(content.padding))
        return on_fail();

    Node *root = Utils::build_huffman_tree(freq);

    uint64_t expected_size = 0;
    for (TYPE_FREQUENCY frequency : freq)
    {
        if (frequency > UINT64_MAX - expected_size)
        {
            delete root;
            return on_fail();
        }
        expected_size += frequency;
    }

    if (archive_version == 2)
    {
        uint64_t archive_size = 0;
        for (const ArchiveEntry &entry : archive_entries)
        {
            if (entry.size > UINT64_MAX - archive_size)
            {
                delete root;
                return on_fail();
            }
            archive_size += entry.size;
        }
        if (archive_size != expected_size)
        {
            delete root;
            return on_fail();
        }
    }
    success = _decode_stream(root, expected_size, content.padding,
                             archive_version == 2 ? &archive_entries : nullptr);

    if (success && archive_version == 2)
    {
        for (const ArchiveSymlink &link : archive_symlinks)
        {
            std::error_code error;
            std::filesystem::create_symlink(link.target, link.output_path, error);
            if (error)
            {
                success = false;
                break;
            }
        }
    }

    delete root;

    writer->close();
    reader->close();

    return success;
}