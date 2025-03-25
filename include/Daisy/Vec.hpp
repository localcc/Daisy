#pragma once

namespace ds {

template <typename T>
struct Vec2 {
    T x;
    T y;
};

template <typename T>
struct Vec3 {
    T x;
    T y;
    T z;
};

template <typename T>
struct Rot {
    T pitch;
    T yaw;
    T roll;
};

template <typename T>
struct Color {
    T r;
    T g;
    T b;
};

} // namespace ds