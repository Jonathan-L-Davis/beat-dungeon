#ifndef LINE_H
#define LINE_H

#include "point.h"

namespace graphics{
    struct line{
        point a,b;
    };
}

static_assert(2*sizeof(graphics::point)==sizeof(graphics::line),"line struct has unexpected size");

#endif//LINE_H
