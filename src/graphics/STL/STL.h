#ifndef STL_H
#define STL_H

#include <string>

namespace STL{
    struct tri{
        float normal[3];
        float p1[3];
        float p2[3];
        float p3[3];
        uint16_t attributes;
    };
    
    struct STL{
        
        bool save(std::string file_path);
        bool load(std::string file_path);
        
        uint8_t header[80];
        uint32_t num_tris;
        tri* tris;// not a vector so we can easily use an arena allocator. Will need a destructor though.
        
        ~STL();
        
    };
}

#endif//STL_H
