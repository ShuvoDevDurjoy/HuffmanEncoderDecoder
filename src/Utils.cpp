#include "../includes/Utils.hpp"
namespace Utils
{

    std::string trim(const std::string &value)
    {
        auto start = std::find_if_not(value.begin(), value.end(), [](unsigned char c)
                                      { return std::isspace(c); });

        auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c)
                                    { return std::isspace(c); })
                       .base();

        if (start >= end)
        {
            return "";
        }

        return std::string(start, end);
    }

    std::string get_extension(const std::filesystem::path &p)
    {
        std::string ext = p.extension().string();
        return ext;
    }

    std::string get_truncated_file_name(const std::filesystem::path &p)
    {
        std::string name = trim(p.stem().string());
        if (name.size() > MAX_FILE_NAME_SIZE)
        {
            name = name.substr(0, MAX_FILE_NAME_SIZE);
        }
        return name;
    }

    std::string get_truncated_file_name_from_path(const std::filesystem::path &p)
    {

        return get_truncated_file_name(p) + get_extension(p);
    }

    std::string to_lower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c)
                       { return static_cast<char>(std::tolower(c)); });
        return value;
    }

    bool file_exists(const std::string &path)
    {
        std::error_code ec;
        return std::filesystem::exists(path, ec) && std::filesystem::is_regular_file(path, ec);
    }

    bool file_exists(const std::filesystem::path &path)
    {
        std::error_code ec;
        return std::filesystem::exists(path, ec) && std::filesystem::is_regular_file(path, ec);
    }

    bool dir_exists(const std::string &path)
    {
        std::error_code ec;
        return std::filesystem::exists(path, ec);
    }

    bool convert_string_to_stream(const std::string content, ByteStream &bs)
    {
        // check if the given string is empty
        // if empty then there are nothing to decode
        if (content.empty())
            return false;
        for (char c : content)
        {
            bs.push_byte(static_cast<uint8_t>(c));
        }

        return true;
    }

    bool convert_stream_to_string(ByteStream &bs, std::string &st)
    {
        if (bs.is_empty())
            return false;
        std::stringstream s;
        Byte byte;
        while (!bs.is_empty() && bs.pop_byte(byte))
        {
            char c = static_cast<char>(byte.get_byte());
            s << c;
        }
        st = s.str();

        return true;
    }

    bool output_parent_exists(const std::string &path)
    {
        std::filesystem::path out_path(path);
        std::filesystem::path parent = out_path.parent_path();

        if (parent.empty())
        {
            return true;
        }

        std::error_code ec;
        return std::filesystem::exists(parent, ec) && std::filesystem::is_directory(parent, ec);
    }

    bool parent_dir(const std::filesystem::path &path, std::filesystem::path &parent_path)
    {
        if (output_parent_exists(path.string()))
        {
            parent_path = path.parent_path();
            return true;
        }
        return false;
    }

    uintmax_t file_size(const std::string &path)
    {
        std::error_code ec;
        uintmax_t size = std::filesystem::file_size(path, ec);
        return ec ? 0 : size;
    }

    std::string format_file_size(uintmax_t bytes)
    {
        const char *units[] = {"B", "KB", "MB", "GB", "TB"};
        double size = static_cast<double>(bytes);
        int unit = 0;

        while (size >= 1024.0 && unit < 4)
        {
            size /= 1024.0;
            unit++;
        }

        std::ostringstream out;
        if (unit == 0)
        {
            out << bytes << " " << units[unit];
        }
        else
        {
            out << std::fixed << std::setprecision(size < 10.0 ? 2 : 1) << size << " " << units[unit];
        }

        return out.str();
    }

    bool encoded_output_path(const std::filesystem::path &input_file, std::string &output_file)
    {
        std::filesystem::path parent_path;
        if (file_exists(input_file) && parent_dir(input_file, parent_path))
        {
            if (parent_path.empty())
            {
                output_file = get_truncated_file_name(input_file) + ENCODER_FILE_EXTENSION;
            }
            else
            {
                output_file = (parent_path / (get_truncated_file_name(input_file) + ENCODER_FILE_EXTENSION)).string();
            }
            return true;
        }
        return false;
    }

    std::string format_size_change(uintmax_t input_size, uintmax_t output_size)
    {
        if (input_size == 0)
        {
            return "n/a";
        }

        double change = (static_cast<double>(output_size) - static_cast<double>(input_size)) * 100.0 / static_cast<double>(input_size);

        std::ostringstream out;
        out << std::fixed << std::setprecision(2);
        if (change > 0)
        {
            out << "+" << change << "%";
        }
        else
        {
            out << change << "%";
        }

        return out.str();
    }

    int byte_count(int num)
    {
        int count = 0;
        while (num > 0)
        {
            num /= 256;
            count++;
        }
        return count;
    }

    bool read_uint8_t(FileReader *reader, std::uint8_t &byte)
    {
        return reader->next_byte(byte);
    }

    bool read_uint16_t(FileReader *reader, uint16_t &bytes)
    {
        uint8_t byte1, byte2;
        if (reader->next_byte(byte1) && reader->next_byte(byte2))
        {
            bytes = 0;
            bytes |= byte1;
            bytes = (bytes << 8) | byte2;
            return true;
        }
        return false;
    }

    bool read_uint64_t(FileReader *reader, uint64_t &f, size_t max_read)
    {
        size_t max_byte_read = 8;
        max_read = std::max(static_cast<size_t>(0), std::min(max_read, max_byte_read));
        uint8_t byte;
        f = 0;
        while (max_read && reader->next_byte(byte))
        {
            f <<= 8;
            f |= byte;
            max_read--;
        }

        return max_read == 0;
    }

    bool read_n_bytes_into_stream(FileReader *reader, ByteStream &bs, size_t n_bytes, bool sync)
    {
        uint8_t byte;
        while (n_bytes && reader->next_byte(byte))
        {
            bs.push_byte(byte, sync);
            n_bytes--;
        }

        return n_bytes == 0;
    }

    uint8_t min_byte_count(uint64_t nc)
    {
        if (nc == 0)
            return 1;
        uint8_t bits = 0;
        uint64_t temp = nc;
        while (temp > 0)
        {
            bits++;
            temp >>= 1;
        }
        return (bits + 7) / 8;
    }

    bool uint16_t_to_bytestream(uint16_t nc, ByteStream &bs)
    {
        uint8_t high_byte = static_cast<uint8_t>((nc >> 8) & 0xFF);
        uint8_t low_byte = static_cast<uint8_t>(nc & 0xFF);
        bs.push_byte(high_byte);
        bs.push_byte(low_byte);
        return true;
    }

    bool read_string(FileReader *reader, std::string &out_string)
    {
        uint8_t size = 0;
        ByteStream stream;
        bool success = Utils::read_uint8_t(reader, size) && read_n_bytes_into_stream(reader, stream, size) && convert_stream_to_string(stream, out_string);
        return success;
    }
    bool read_string2(FileReader *reader, std::string &out_string)
    {
        uint16_t size = 0;
        ByteStream stream;
        bool success = Utils::read_uint16_t(reader, size) && read_n_bytes_into_stream(reader, stream, size) && convert_stream_to_string(stream, out_string);
        return success;
    }

    bool frequency_to_stream(std::array<TYPE_FREQUENCY, BYTE_SIZE> &freq, ByteStream &bs, uint8_t min_bytes)
    {
        Terminal::stat_carriage("Converting Frequecy Bytes to Stream", Status::Modern::PROGRESS);
        for (size_t i = 0; i < freq.size(); ++i)
        {
            if (freq[i])
            {
                bs.push_byte(static_cast<uint8_t>(i));
                for (int x = min_bytes - 1; x >= 0; --x)
                {
                    uint8_t byte = (freq[i] >> (8 * x)) & 0xFF;
                    bs.push_byte(byte);
                }
            }
        }

        Terminal::stat_carriage("Converting Frequecy Bytes to Stream", Status::Modern::SUCCESS);
        std::cout << std::endl;
        return bs.size() != 0;
    }

    void build_frequency_map(std::array<TYPE_FREQUENCY, BYTE_SIZE> &freq, uint16_t &num_unique_chars, uint8_t &min_num_bytes, FileReader *reader, bool show_terminal)
    {
        // set initial frequency of all byte to 0
        freq.fill(0);
        num_unique_chars = 0;
        // variable to load next byte in
        TYPE_CHAR next_byte;
        // reader reads a file till end to build frequency map

        size_t read = 0;
        size_t step_size = 0;
        uint8_t last_progress = 0;
        size_t total_bytes = reader->size();
        if (show_terminal)
        {
            Terminal::run_with_progress("Reading File", last_progress);
            step_size = std::max<size_t>(1, total_bytes / 100);
        }

        while (reader->next_byte(next_byte))
        {
            if (freq[next_byte] == 0)
            {
                num_unique_chars++;
            }
            freq[next_byte]++;
            read++;
            if (show_terminal && (read % step_size == 0))
            {
                uint8_t current_progress = static_cast<uint8_t>((read * 100) / total_bytes);

                // Only update terminal UI if percentage actually changed
                if (current_progress > last_progress)
                {
                    Terminal::run_with_progress("Reading File", current_progress);
                    last_progress = current_progress;
                }
            }
        }

        if (show_terminal)
        {
            Terminal::run_with_progress("Reading File", 100);
            std::cout << std::endl;
        }

        auto max_freq_size = std::max_element(freq.begin(), freq.end());
        uint64_t max_frequency = *max_freq_size;

        min_num_bytes = min_byte_count(max_frequency);
    }

    Node *build_huffman_tree(std::array<TYPE_FREQUENCY, BYTE_SIZE> &freq)
    {
        Terminal::stat_carriage("Building Huffman Tree: ", Status::Modern::PROGRESS);
        std::priority_queue<Node *, std::vector<Node *>, Compare> min_heap;
        // initialize the min heap with leaf nodes
        for (size_t byte = 0; byte < freq.size(); ++byte)
        {
            // check if the frequency of the byte is not 0
            if (freq[byte])
            {
                Node *node = new Node(byte, freq[byte]);
                min_heap.push(node);
            }
        }

        if (min_heap.size() == 0)
            return nullptr;

        // untill the min heap is of size 1, means only the root node of the tree is left
        // retrieve two smaller frequency nodes and create a new node with frequency equal to their sum
        // and push that into the mean heap
        while (min_heap.size() > 1)
        {
            // get the left node
            Node *left = min_heap.top();
            min_heap.pop();
            // get the right node
            Node *right = min_heap.top();
            min_heap.pop();

            // insert a new node of frequency equals sum of left and right node
            min_heap.push(new Node(left->frequency + right->frequency, left, right));
        }

        // At this point top value of min heap is the root node of the huffman tree
        Node *root = min_heap.top();
        min_heap.pop();
        Terminal::stat_carriage("Buiding Huffman Tree: ", Status::Modern::SUCCESS);
        std::cout << std::endl;

        // return the root node
        return root;
    }

    // performs dfs on the huffman tree until a leaf node is found
    // when a leaf node is found then set the code for the byte
    // it is guaranted that leaf nodes are byte node by initialization of priority queue
    void build_huffman_codes(Node *root, ByteStream &code, std::array<ByteStream, BYTE_SIZE> &huffman_codes)
    {
        // check if root is null or not
        // this is just a guard against malicious input
        if (root == nullptr)
            return;
        if (root->is_leaf())
        {
            if (code.is_empty())
            {
                code.push_bit(false);
            }
            huffman_codes[root->c] = code;
            return;
        }
        bool bit;

        // when we will go left then bit 0 will be inserted into code
        // and when we will go right then bit 1 will be inserted into code
        // after going left when we will go right then bit 0 will popped out and
        // same for after doing right traversal(pop bit 1)

        // go left
        code.push_bit(false);
        build_huffman_codes(root->left, code, huffman_codes);
        // back from left
        code.pop_bit(bit);
        // go right
        code.push_bit(true);
        build_huffman_codes(root->right, code, huffman_codes);
        // back from right
        code.pop_bit(bit);
    }
};
