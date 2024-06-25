#include "math/math.h"

#include <math.h>

Mat4
mat4_identity(void)
{
  return (Mat4){1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
}

Mat4
mat4_perspective(F32 fov, F32 aspect, F32 near, F32 far)
{
  F32 tan_half_fov = tan(fov / 2);

  Mat4 out = {0};
  out.m00  = 1 / (aspect * tan_half_fov);
  out.m11  = 1 / tan_half_fov;
  out.m22  = -(far + near) / (far - near);
  out.m23  = -1;
  out.m32  = -(2 * far * near) / (far - near);

  return out;
}

Mat4
mat4_lookat(Vec3 eye, Vec3 center, Vec3 up)
{
  Mat4 out = mat4_identity();

  Vec3 f = vec3_norm(vec3_sub(center, eye));
  Vec3 s = vec3_norm(vec3_cross(f, up));
  Vec3 u = vec3_cross(s, f);

  out.m00 = s.x;
  out.m10 = s.y;
  out.m20 = s.z;
  out.m01 = u.x;
  out.m11 = u.y;
  out.m21 = u.z;
  out.m02 = -f.x;
  out.m12 = -f.y;
  out.m22 = -f.z;
  out.m30 = -vec3_dot(s, eye);
  out.m31 = -vec3_dot(u, eye);
  out.m32 = vec3_dot(f, eye);

  return out;
}
