#ifndef TRI_H
#define TRI_H

#include "point.h"

namespace graphics{
    struct tri{
        point a,b,c;
    };
}

static_assert(3*sizeof(graphics::point)==sizeof(graphics::tri),"tri struct has unexpected size");

#endif//TRI_H
