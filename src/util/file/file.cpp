#include "file.h"

#include <cstdio>
#include <filesystem>

std::vector<uint8_t> load_file(std::string file_path){
    std::vector<uint8_t> retMe;
    
    int64_t file_size = std::filesystem::file_size(file_path);
    
    // using the C api because windows likes to insert carriage returns, even in binary mode.
    FILE* file = fopen(file_path.c_str(),"rb");
    
    retMe.resize(file_size);
    
    for( int64_t bytes_read = 0; bytes_read < file_size && !std::feof(file); bytes_read += std::fread((char*)retMe.data()+bytes_read,1,file_size,file) );
    
    // return empty object on error/incomplete file read. Don't want to deal with half reading in a file.
    if(file_size!=retMe.size())
        retMe = {};
        
    fclose(file);
    
    return retMe;
}

bool save_file(std::string file_path, const std::vector<uint8_t>& data){
        FILE* file = fopen(file_path.c_str(),"rb");
        
        std::size_t d = fwrite((char*)data.data(),1,data.size(),file);
        
        fclose(file);
        
        return data.size()==d;// technically not the most robust way to save files. Could also use a forloop, which would be proper, but ehh.
}
