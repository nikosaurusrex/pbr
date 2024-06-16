#include "math.h"

#include <math.h>

Vec3
vec3(float x, float y, float z)
{
    return (Vec3){x, y, z};
}

Vec3
vec3_sub(Vec3 v1, Vec3 v2)
{
    return (Vec3){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

float
vec3_length(Vec3 v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vec3
vec3_norm(Vec3 v)
{
    float f = 1.0f / vec3_length(v);

    return (Vec3){v.x * f, v.y * f, v.z * f};
}

Vec3
vec3_cross(Vec3 v1, Vec3 v2)
{
    float x = v1.y * v2.z - v2.y * v1.z;
    float y = v1.z * v2.x - v2.z * v1.x;
    float z = v1.x * v2.y - v2.x * v1.y;

    return (Vec3){x, y, z};
}

float
vec3_dot(Vec3 v1, Vec3 v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
