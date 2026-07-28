#include "../includes/FileReader.hpp"

bool FileReader::get_file_path(std::filesystem::path &path)
{
    if (is_open())
    {
        path = this->file_path;
        return true;
    }

    return false;
}

bool FileReader::reset()
{
    if (!is_open())
    {
        return false;
    }
    file.clear();
    file.seekg(0, std::ios::beg);
    bytes_read = 0;
    index = 0;
    return !file.fail() && file.tellg() == std::streampos(0);
}

size_t FileReader::size()
{
    if (is_open())
    {
        return std::filesystem::file_size(this->file_path);
    }
    return 0;
}

bool FileReader::open(std::string file_name){
    if(!Utils::file_exists(file_name))
        return false;
    file.open(file_name, std::ios::in | std::ios::binary);
    this->file_path = std::filesystem::path(file_name);
    bytes_read = 0;
    index = 0;
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
        file_path = "";
    }
}

bool FileReader::is_buffer_empty(){
    return bytes_read == 0;
}

bool FileReader::can_read(){
    bool can_read = !is_buffer_empty() || (is_open() && (!file.eof()));
    if(can_read){
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