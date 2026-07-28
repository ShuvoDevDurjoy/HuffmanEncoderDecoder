#include <iostream>
#include <string>

#include "includes/Huffman.hpp"
#include "includes/Terminal.hpp"

namespace
{
    const int EXIT_USAGE = 1;
    const int EXIT_OPERATION_FAILED = 2;

    void print_usage()
    {
        Terminal::banner();
        Terminal::section("Usage");
        std::cout << "  huff encode <file>\n";
        std::cout << "  huff decode <file.huff> <output-file>\n";
        std::cout << "  huff\n\n";
        Terminal::info("Run without arguments to start interactive mode.");
    }

    bool validate_input_file(const std::string &input_file)
    {
        if(input_file.empty())
        {
            Terminal::error("input file path is required");
            return false;
        }

        if(!Utils::file_exists(input_file))
        {
            Terminal::error("input file not found: " + input_file);
            return false;
        }

        if(Utils::file_size(input_file) == 0)
        {
            Terminal::error("empty files are not supported by this encoder");
            return false;
        }

        return true;
    }

    bool validate_output_file(const std::string &output_file)
    {
        if(output_file.empty())
        {
            Terminal::error("output file path is required");
            return false;
        }

        if(!Utils::output_parent_exists(output_file))
        {
            Terminal::error("output directory does not exist: " + output_file);
            return false;
        }

        return true;
    }

    void print_start_summary(const std::string &mode, const std::string &input_file,
                             const std::string &output_file)
    {
        Terminal::section(mode);
        Terminal::stat("Input", input_file);
        Terminal::stat("Output", output_file);
        Terminal::stat("Input Size", Utils::format_file_size(Utils::file_size(input_file)));

        if(Utils::file_exists(output_file))
        {
            Terminal::warning("output file exists and will be overwritten");
        }
    }

    void print_result_summary(const std::string &input_file, const std::string &output_file)
    {
        uintmax_t input_size = Utils::file_size(input_file);
        uintmax_t output_size = Utils::file_size(output_file);

        Terminal::section("Result");
        Terminal::stat("Input Size", Utils::format_file_size(input_size));
        Terminal::stat("Output Size", Utils::format_file_size(output_size));
        Terminal::stat("Size Change", Utils::format_size_change(input_size, output_size));
        Terminal::stat("Output File", output_file);
    }

    bool encode_file(std::string input_file)
    {
        input_file = Utils::trim(input_file);
        std::string output_file = "";

        bool success;
        success = Utils::encoded_output_path(input_file, output_file);

        if(!validate_input_file(input_file) || !validate_output_file(output_file))
            return false;

        Terminal::banner();
        print_start_summary("Encode", input_file, output_file);

        Huffman huffman;
        // bool success = Terminal::run_with_progress("Encoding", [&](){
        //     return huffman.encode(input_file, output_file);
        // });
        success = huffman.encode(input_file, output_file);

        if (!success || !Utils::file_exists(output_file))
        {
            Terminal::error("encoding failed");
            return false;
        }

        Terminal::success("encoding complete");
        print_result_summary(input_file, output_file);
        return true;
    }

    bool decode_file(std::string input_file, std::string output_file)
    {
        input_file = Utils::trim(input_file);
        output_file = Utils::trim(output_file);

        if(!validate_input_file(input_file) || !Utils::dir_exists(output_file))
            return false;

        Terminal::banner();
        print_start_summary("Decode", input_file, output_file);

        Huffman huffman;
        std::string out_file_name;
        bool success = huffman.decode(input_file, output_file, out_file_name);

        if(!success)
        {
            Terminal::error("decoding failed");
            return false;
        }

        Terminal::success("decoding complete");
        print_result_summary(input_file, out_file_name);
        return true;
    }

    bool interactive_mode()
    {
        Terminal::banner();

        while(true)
        {
            Terminal::section("Interactive Mode");
            std::cout << "  1. Encode a file\n";
            std::cout << "  2. Decode a .huff file\n";
            std::cout << "  0. Exit\n\n";

            std::string choice = Utils::to_lower(Utils::trim(Terminal::prompt("Select option:")));
            if(std::cin.eof())
                return true;

            if(choice == "0" || choice == "q" || choice == "quit" || choice == "exit")
                return true;

            if(choice == "1" || choice == "e" || choice == "encode")
            {
                std::string input_file = Terminal::prompt("Input file:");
                encode_file(input_file);
                std::cout << "\n";
                continue;
            }

            if(choice == "2" || choice == "d" || choice == "decode")
            {
                std::string input_file = Terminal::prompt("Encoded file:");
                std::string output_file = Terminal::prompt("Output file:");
                decode_file(input_file, output_file);
                std::cout << "\n";
                continue;
            }

            Terminal::error("unknown option: " + choice);
            std::cout << "\n";
        }
    }
}

int main(int argc, char* argv[])
{
    Terminal::init();

    if(argc == 1)
    {
        return interactive_mode() ? 0 : EXIT_OPERATION_FAILED;
    }

    std::string command = Utils::to_lower(argv[1]);

    if(command == "-h" || command == "--help" || command == "help")
    {
        print_usage();
        return 0;
    }

    if(command == "encode")
    {
        if(argc != 3)
        {
            print_usage();
            return EXIT_USAGE;
        }

        return encode_file(argv[2]) ? 0 : EXIT_OPERATION_FAILED;
    }

    if(command == "decode")
    {
        if(argc != 4)
        {
            print_usage();
            return EXIT_USAGE;
        }

        return decode_file(argv[2], argv[3]) ? 0 : EXIT_OPERATION_FAILED;
    }

    Terminal::error("unknown command: " + command);
    print_usage();
    return EXIT_USAGE;
}
