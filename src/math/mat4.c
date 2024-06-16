#include "math/math.h"

#include <math.h>

void
mat4_identity(Matrix4f *out)
{
    *out = (Matrix4f){1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
}

void
mat4_perspective(Matrix4f *out, float fov, float aspect, float near, float far)
{
    float tan_half_fov = tan(fov / 2);

    *out = (Matrix4f){0};
    out->m00 = 1 / (aspect * tan_half_fov);
    out->m11 = 1 / tan_half_fov;
    out->m22 = -(far + near) / (far - near);
    out->m23 = -1;
    out->m32 = -(2 * far * near) / (far - near);
}
