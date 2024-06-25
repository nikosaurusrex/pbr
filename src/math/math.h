#pragma once

#include "base/base.h"

typedef struct Vec2 Vec2;
typedef union Vec3  Vec3;
typedef struct Vec4 Vec4;
typedef struct Mat4 Mat4;

C_LINKAGE_BEGIN

struct Vec2 {
  F32 x;
  F32 y;
};

union Vec3 {
  struct {
    F32 x;
    F32 y;
    F32 z;
  };
  struct {
    Vec2 xy;
    F32  _notused1;
  };
  struct {
    F32  _notused2;
    Vec2 yz;
  };
};

struct Vec4 {
  F32 x;
  F32 y;
  F32 z;
  F32 w;
};

struct Mat4 {
  union {
    F32 m[16];
    struct {
      F32 m00, m01, m02, m03;
      F32 m10, m11, m12, m13;
      F32 m20, m21, m22, m23;
      F32 m30, m31, m32, m33;
    };
  };
};

Vec2 vec2(F32 x, F32 y);

Vec3 vec3(F32 x, F32 y, F32 z);
Vec3 vec3_sub(Vec3 v1, Vec3 v2);
F32  vec3_length(Vec3 v);
Vec3 vec3_norm(Vec3 v);
Vec3 vec3_cross(Vec3 v1, Vec3 v2);
F32  vec3_dot(Vec3 v1, Vec3 v2);
void vec3_print(Vec3 v);

Vec4 vec4(F32 x, F32 y, F32 z, F32 w);

Mat4 mat4_identity(void);
Mat4 mat4_perspective(F32 fov, F32 aspect, F32 near, F32 far);
Mat4 mat4_lookat(Vec3 eye, Vec3 center, Vec3 up);

C_LINKAGE_END
