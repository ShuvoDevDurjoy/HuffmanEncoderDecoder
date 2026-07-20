#include "../includes/FileReader.hpp"

bool FileReader::open(std::string file_name){
    file.open(file_name, std::ios::binary);
    bytes_read = 0;
    return is_open();
}

bool FileReader::is_open() const{
    return file.is_open();
}

bool FileReader::read_chunk()
{
    file.read(reinterpret_cast<char *>(buffer.data()), BUFFER_SIZE);
    bytes_read = file.gcount();
    index = 0;
    // std::cout << "Buffer Read: " << bytes_read << std::endl;
    return bytes_read > 0;
}

void FileReader::close(){
    if(file.is_open()){
        this->file.close();
    }
}

bool FileReader::is_buffer_empty(){
    return bytes_read == 0;
}

bool FileReader::can_read(){
    bool can_read = !is_buffer_empty() || (is_open() && (!file.eof()));
    if (!can_read)
    {
        close();
    }
    else{
        if(is_buffer_empty()){
            return read_chunk();
        }
        return true;
    }
    return false;
}

bool FileReader::next_byte(uint8_t &byte){
    if(can_read()){
        byte = buffer[index++];
        bytes_read--;
        return true;
    }
    return false;
}