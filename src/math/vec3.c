#include "math.h"

#include <stdio.h>
#include <math.h>

Vec3
vec3(f32 x, f32 y, f32 z)
{
    return (Vec3){x, y, z};
}

Vec3
vec3_sub(Vec3 v1, Vec3 v2)
{
    return (Vec3){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

f32
vec3_length(Vec3 v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vec3
vec3_norm(Vec3 v)
{
    f32 f = 1.0f / vec3_length(v);

    return (Vec3){v.x * f, v.y * f, v.z * f};
}

Vec3
vec3_cross(Vec3 v1, Vec3 v2)
{
    f32 x = v1.y * v2.z - v2.y * v1.z;
    f32 y = v1.z * v2.x - v2.z * v1.x;
    f32 z = v1.x * v2.y - v2.x * v1.y;

    return (Vec3){x, y, z};
}

f32
vec3_dot(Vec3 v1, Vec3 v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

void
vec3_print(Vec3 v)
{
    printf("(%f, %f, %f\n", v.x, v.y, v.z);
}
