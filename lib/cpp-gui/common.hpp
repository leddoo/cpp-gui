#pragma once


#include <cpp-common/common.hpp>
#include <cpp-common/math.hpp>


#ifdef UNICODE
#undef UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


#include <d2d1.h>
#include <dwrite.h>

template <typename T>
void safe_release(T** object) {
    if((*object) != nullptr) {
        (*object)->Release();
        (*object) = nullptr;
    }
}


inline D2D_POINT_2F to_d2d_point2f(V2f v) { return { v.x, v.y }; }
inline D2D_SIZE_F   to_d2d_sizef(V2f v)   { return { v.x, v.y }; }
inline D2D_COLOR_F  to_d2d_colorf(V4f v)  { return { v.r, v.g, v.b, v.a }; }



enum class Axis : Uint8 { x = 0, y = 1, };

inline Axis get_cross_axis(Axis axis) {
    return Axis(1 - Uint8(axis));
}

inline V2f make_unit(Axis axis) {
    return V2f { Float32(axis == Axis::x), Float32(axis == Axis::y) };
}


enum class Direction : Uint8 { min = 0, max = 1, };

inline Direction get_other_direction(Direction direction) {
    return Direction(1 - Uint8(direction));
}

inline V2f make_unit(Axis axis, Direction direction) {
    auto scale = 2.0f*Float32(direction) - 1.0f;
    return scale * make_unit(axis);
}


struct Box_Constraints {
    V2f min;
    V2f max;


    static Box_Constraints tight(V2f size) {
        return Box_Constraints { size, size };
    }
};

