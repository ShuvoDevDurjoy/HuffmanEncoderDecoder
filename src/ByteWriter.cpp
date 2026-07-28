#include "../includes/ByteWriter.hpp"


bool ByteWriter::_write(Byte &byte){
        _write(byte.get_byte());
        return true;
    }

    bool ByteWriter::_write(uint8_t byte){
        if(is_open()){
            file.write(reinterpret_cast<const char *>(&byte), sizeof(byte));
            return file.good();
        }
        return false;
    }


    bool ByteWriter::open(std::string file_name){
        file.open(file_name, std::ios::out | std::ios::binary);
        return is_open();
    }

    bool ByteWriter::is_open(){
        return file.is_open();
    }




void ByteWriter::close(){
        if(is_open()){
            file.close();
        }
    }


bool ByteWriter::write(Byte &b){
        return _write(b);
    }

    bool ByteWriter::write(uint8_t &c){
        return _write(c);
    }


bool ByteWriter::write(ByteStream &bs, bool show_terminal)
    {
        size_t read = 0;
        size_t step_size = 0;
        uint8_t last_progress = 0;
        size_t total_bytes = bs.size();
        if (show_terminal)
        {
            Terminal::run_with_progress("Writing into File", last_progress);
            step_size = std::max<size_t>(1, total_bytes / 100);
        }
        while (!bs.is_empty())
        {
            Byte b;
            bs.pop_byte(b);
            this->_write(b);
            ++read;
            if (show_terminal && (read % step_size == 0))
            {
                uint8_t current_progress = static_cast<uint8_t>((read * 100) / total_bytes);

                // Only update terminal UI if percentage actually changed
                if (current_progress > last_progress)
                {
                    Terminal::run_with_progress("Writing into File", current_progress);
                    last_progress = current_progress;
                }
            }
        }
        if(show_terminal){
            Terminal::run_with_progress("Writing into File", 100);
            std::cout << std::endl;
        }
        return true;
    }