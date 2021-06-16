#pragma once


#include <functional>
using Void_Callback = std::function<void(void)>;


#include <cpp-common/common.hpp>
#include <cpp-common/math.hpp>


using Win32_Virtual_Key = Uint8;
using Ascii_Char = Uint8;



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



enum Mouse_Button : Uint8 {
    left,
    middle,
    right,

    _count
};

