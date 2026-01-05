#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "common.h"

#include "geometry/tri.h"
#include "geometry/line.h"
#include "geometry/point.h"
#include "vulkan/shader.h"

namespace graphics{
    bool initialize();
    
    bool submit_tris(std::vector<tri>&,shaderID);
    bool submit_lines(std::vector<line>&,shaderID);
    bool submit_points(std::vector<point>&,shaderID);
    
    void start_frame();
    void end_frame();
    
    bool terminate();
}

#endif//GRAPHICS_H

