#pragma once

#include "base/base.h"

typedef struct Vec2 Vec2;
typedef union Vec3  Vec3;
typedef struct Vec4 Vec4;
typedef struct Mat4 Mat4;

C_LINKAGE_BEGIN

struct Vec2 {
    f32 x;
    f32 y;
};

union Vec3 {
    struct {
        f32 x;
        f32 y;
        f32 z;
    };
    struct {
        Vec2 xy;
        f32  _notused1;
    };
    struct {
        f32  _notused2;
        Vec2 yz;
    };
};

struct Vec4 {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
};

struct Mat4 {
    union {
        f32 m[16];
        struct {
            f32 m00, m01, m02, m03;
            f32 m10, m11, m12, m13;
            f32 m20, m21, m22, m23;
            f32 m30, m31, m32, m33;
        };
    };
};

Vec2 vec2(f32 x, f32 y);

Vec3 vec3(f32 x, f32 y, f32 z);
Vec3 vec3_sub(Vec3 v1, Vec3 v2);
f32  vec3_length(Vec3 v);
Vec3 vec3_norm(Vec3 v);
Vec3 vec3_cross(Vec3 v1, Vec3 v2);
f32  vec3_dot(Vec3 v1, Vec3 v2);

Vec4 vec4(f32 x, f32 y, f32 z, f32 w);

Mat4 mat4_identity(void);
Mat4 mat4_perspective(f32 fov, f32 aspect, f32 near, f32 far);
Mat4 mat4_lookat(Vec3 eye, Vec3 center, Vec3 up);

C_LINKAGE_END
