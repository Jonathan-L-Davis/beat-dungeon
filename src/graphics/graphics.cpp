#include "graphics.h"

#include "vulkan/shader.h"
#include "vulkan/vulkan.h"

namespace graphics{
    
    bool initialize(){
        return graphics::vulkan::initialize();
    }
    
    bool submit_tris(std::vector<tri>&,shaderID){
        return false;
    }
    
    bool submit_lines(std::vector<line>&,shaderID){
        return false;
    }
    
    bool submit_points(std::vector<point>&,shaderID){
        return false;
    }
    
    void start_frame(){
        graphics::vulkan::start_frame();
    }
    
    void end_frame(){
        graphics::vulkan::end_frame();
    }
    
    bool terminate(){
        return graphics::vulkan::terminate();
    }
    
}
