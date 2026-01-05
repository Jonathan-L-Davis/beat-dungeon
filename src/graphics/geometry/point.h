#ifndef POINT_H
#define POINT_H

namespace graphics{
    struct point{
        float position[4];
        float color[4];
        uint32_t uv[4];
    };
}

static_assert(sizeof(graphics::point)==48,"Point struct has unexpected size");

#endif//POINT_H
