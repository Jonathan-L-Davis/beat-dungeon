#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "common.h"

#include "geometry/tri.h"
#include "geometry/line.h"
#include "geometry/point.h"
#include "vulkan/vulkan.h"

namespace graphics{
    bool initialize();
    
    void start_frame();
    void end_frame();
    
    bool terminate();
}

#endif//GRAPHICS_H

