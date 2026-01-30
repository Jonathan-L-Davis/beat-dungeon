#include "file.h"

#include <cstdio>
#include <filesystem>

std::vector<uint8_t> load_file(std::string file_path){
    std::vector<uint8_t> retMe;
    if( !std::filesystem::exists(file_path)||!std::filesystem::is_regular_file(file_path) ) return {};
    int64_t file_size = std::filesystem::file_size(file_path);
    
    // using the C api because windows likes to insert carriage returns, even in binary mode.
    FILE* file = fopen(file_path.c_str(),"rb");
    if(file==NULL) return {};
    retMe.resize(file_size);
    
    for( int64_t bytes_read = 0; bytes_read < file_size && !std::feof(file); bytes_read += std::fread((char*)retMe.data()+bytes_read,1,file_size,file) );
    
    // return empty object on error/incomplete file read. Don't want to deal with half reading in a file.
    if(file_size!=retMe.size())
        retMe = {};
    
    fclose(file);
    
    return retMe;
}

bool save_file(std::string file_path, const std::vector<uint8_t>& data){
        FILE* file = fopen(file_path.c_str(),"wb");
        
        if(file==NULL) return false;
        
        std::size_t d = fwrite((char*)data.data(),1,data.size(),file);
        
        fclose(file);
        
        return data.size()==d;// technically not the most robust way to save files. Could also use a forloop, which would be proper, but ehh.
}

void append_to_buffer(std::vector<uint8_t>& file, uint8_t x){
    file.push_back(x>>0);
}

void append_to_buffer(std::vector<uint8_t>& file, uint16_t x){
    file.push_back(x>>8);
    file.push_back(x>>0);
}

void append_to_buffer(std::vector<uint8_t>& file, uint32_t x){
    file.push_back(x>>24);
    file.push_back(x>>16);
    file.push_back(x>>8);
    file.push_back(x>>0);
}

void append_to_buffer(std::vector<uint8_t>& file, uint64_t x){
    file.push_back(x>>56);
    file.push_back(x>>48);
    file.push_back(x>>40);
    file.push_back(x>>32);
    file.push_back(x>>24);
    file.push_back(x>>16);
    file.push_back(x>>8);
    file.push_back(x>>0);
}

void read_from_buffer(std::vector<uint8_t> file, int& i, uint8_t& x){
    if(file.size()<=i+0) return;
    
    x = 0;
    
    x |= file[i]; i++;
}

void read_from_buffer(std::vector<uint8_t> file, int& i, uint16_t& x){
    if(file.size()<=i+1) return;
    
    x = 0;
    
    x <<= 8; x |= file[i]; i++;
    x <<= 8; x |= file[i]; i++;
}

void read_from_buffer(std::vector<uint8_t> file, int& i, uint32_t& x){
    if(file.size()<=i+3) return;
    
    x = 0;
    
    x <<= 8; x |= file[i]; i++;
    x <<= 8; x |= file[i]; i++;
    x <<= 8; x |= file[i]; i++;
    x <<= 8; x |= file[i]; i++;
}

void read_from_buffer(std::vector<uint8_t> file, int& i, uint64_t& x){
    if(file.size()<=i+7) return;
    
    x = 0;
    
    x <<= 8; x |= file[i]; i++;
    x <<= 8; x |= file[i]; i++;
    x <<= 8; x |= file[i]; i++;
    x <<= 8; x |= file[i]; i++;
    x <<= 8; x |= file[i]; i++;
    x <<= 8; x |= file[i]; i++;
    x <<= 8; x |= file[i]; i++;
    x <<= 8; x |= file[i]; i++;
}
