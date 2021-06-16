#pragma once

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

