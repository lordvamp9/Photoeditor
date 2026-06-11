#pragma once

#include "core/types.hpp"

#include <algorithm>
#include <cmath>

namespace PhotoStudio::Math {

template <typename T>
inline T lerp(const T& a, const T& b, f32 t)
{
    return a + (b - a) * t;
}

inline f32 smoothstep(f32 edge0, f32 edge1, f32 x)
{
    const f32 t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// Catmull-Rom spline through p1..p2 with neighbors p0 / p3.
inline f32 catmullRom(f32 p0, f32 p1, f32 p2, f32 p3, f32 t)
{
    const f32 t2 = t * t;
    const f32 t3 = t2 * t;
    return 0.5f * ((2.0f * p1) + (-p0 + p2) * t + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
                   (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

} // namespace PhotoStudio::Math
