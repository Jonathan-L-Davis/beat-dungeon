#include "STL.h"
#include <fstream>

#include "common.h"

namespace STL{
    
    namespace details{
        
        union float2uint32{
            float f;
            uint32_t u;
            /// if these fail, you're compiling for a very strange platform.
            static_assert(sizeof(float)==4,"Need floats to be 4 bytes.");
            static_assert(sizeof(uint32_t)==4,"Need uint32_t to be 4 bytes.");
        };
        
        bool read_int16(std::fstream& file, uint16_t& readMe){
            uint8_t b1,b2;
            
            if(!file.read((char*)&b1,1))
                return false;
            if(!file.read((char*)&b2,1))
                return false;
            
            readMe = (uint16_t(b2)<<8)|(uint16_t(b1)<<0);
            
            return true;
        }
        
        bool read_int32(std::fstream& file, uint32_t& readMe){
            uint8_t b1,b2,b3,b4;
            
            if(!file.read((char*)&b1,1))
                return false;
            if(!file.read((char*)&b2,1))
                return false;
            if(!file.read((char*)&b3,1))
                return false;
            if(!file.read((char*)&b4,1))
                return false;
            
            readMe = (uint32_t(b4)<<24)|(uint32_t(b3)<<16)|(uint32_t(b2)<<8)|(uint32_t(b1)<<0);
            
            return true;
        }
        
        bool read_float(std::fstream& file, float& readMe){
            uint8_t b1,b2,b3,b4;
            
            if(!file.read((char*)&b1,1))
                return false;
            if(!file.read((char*)&b2,1))
                return false;
            if(!file.read((char*)&b3,1))
                return false;
            if(!file.read((char*)&b4,1))
                return false;
            
            float2uint32 p;
            p.u = (uint32_t(b4)<<24)|(uint32_t(b3)<<16)|(uint32_t(b2)<<8)|(uint32_t(b1)<<0);
            readMe = p.f;
            
            return true;
        }
        
        bool read_tri(std::fstream& file, tri& readMe){
            
            for(int i = 0; i < 3; i++)
                if( !read_float(file,readMe.normal[i]) )
                    return false;
            
            for(int i = 0; i < 3; i++)
                if( !read_float(file,readMe.p1[i]) )
                    return false;
            
            for(int i = 0; i < 3; i++)
                if( !read_float(file,readMe.p2[i]) )
                    return false;
            
            for(int i = 0; i < 3; i++)
                if( !read_float(file,readMe.p3[i]) )
                    return false;
            
            if(!read_int16(file,readMe.attributes))
                return false;
            
            return true;
        }
        
        bool write_int16(std::fstream& file, const uint32_t& writeMe){
            uint16_t p;
            p = writeMe;
            uint8_t b1,b2;
            
            b1 = (p>> 0)&0xFF;
            b2 = (p>> 8)&0xFF;
            
            p = (uint32_t(b2)<<8)|(uint32_t(b1)<<0);
            
            if(!file.write((char*)&p,2))
                return false;
            
            return true;
        }
        
        bool write_int32(std::fstream& file, const uint32_t& writeMe){
            float2uint32 p;
            p.u = writeMe;
            uint8_t b1,b2,b3,b4;
            
            b1 = (p.u>> 0)&0xFF;
            b2 = (p.u>> 8)&0xFF;
            b3 = (p.u>>16)&0xFF;
            b4 = (p.u>>24)&0xFF;
            
            p.u = (uint32_t(b4)<<24)|(uint32_t(b3)<<16)|(uint32_t(b2)<<8)|(uint32_t(b1)<<0);
            
            if(!file.write((char*)&p.u,4))
                return false;
            
            return true;
        }
        
        bool write_float(std::fstream& file, const float& writeMe){
            float2uint32 p;
            p.f = writeMe;
            uint8_t b1,b2,b3,b4;
            
            b1 = (p.u>> 0)&0xFF;
            b2 = (p.u>> 8)&0xFF;
            b3 = (p.u>>16)&0xFF;
            b4 = (p.u>>24)&0xFF;
            
            p.u = (uint32_t(b4)<<24)|(uint32_t(b3)<<16)|(uint32_t(b2)<<8)|(uint32_t(b1)<<0);
            
            if(!file.write((char*)&p.f,4))
                return false;
            
            return true;
        }
        
        bool write_tri(std::fstream& file, const tri& writeMe){
            
            for(int i = 0; i < 3; i++)
                if( !write_float(file,writeMe.normal[i]) )
                    return false;
            
            for(int i = 0; i < 3; i++)
                if( !write_float(file,writeMe.p1[i]) )
                    return false;
            
            for(int i = 0; i < 3; i++)
                if( !write_float(file,writeMe.p2[i]) )
                    return false;
            
            for(int i = 0; i < 3; i++)
                if( !write_float(file,writeMe.p3[i]) )
                    return false;
            
            if(!write_int16(file,writeMe.attributes))
                return false;
            
            return true;
        }
    }
    
    bool STL::load(std::string file_path){
        if(!std::filesystem::exists(file_path)||!std::filesystem::is_regular_file(file_path))
            return false;
        
        int64_t file_size = std::filesystem::file_size(file_path);
        
        if(file_size < 84)
            return false;
        
        /// Using a file stream for binary assumes we're not on windows/linking to the windows runtime. MSVC runtime adds carriage returns to ANY newline character, even if you're trying to read/write in binary mode.
        std::fstream file(file_path,std::ios::in|std::ios::binary);
        
        for(int i = 0; i < 80; i++){
            uint8_t b;
            if(!file.read((char*)&b,1))
                return false;
            header[i] = b;
        }
        
        if(!details::read_int32(file,num_tris))
            return false;
        
        if( file_size!=(84+50*num_tris) )
            return false;
        
        // going to be replaced with an arena allocation once I implement that.
        tris = (tri*)malloc(sizeof(tri)*num_tris);
        
        for(int i = 0; i < num_tris; i++){
            if(!details::read_tri(file,tris[i]))
                return false;
        }
        
        return true;
    }
    
    bool STL::save(std::string file_path){
        
        /// Using a file stream for binary assumes we're not on windows/linking to the windows runtime. MSVC runtime adds carriage returns to ANY newline character, even if you're trying to read/write in binary mode.
        std::fstream file(file_path,std::ios::out|std::ios::binary);
        
        if(!file.write((char*)&header,80))
            return false;
        
        if(!details::write_int32(file,num_tris))
            return false;
        
        for(int i = 0; i < num_tris; i++){
            if(!details::write_tri(file,tris[i]))
                return false;
        }
        
        return true;
    }
    
    STL::~STL(){
        if(tris!=nullptr)
            free(tris);
        tris = nullptr;
    }
    
}
