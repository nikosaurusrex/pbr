#pragma once

typedef struct Vec2 Vec2;
typedef union Vec3  Vec3;
typedef struct Mat4 Mat4;

#ifdef __cplusplus
extern "C" {
#endif

struct Vec2 {
    float x;
    float y;
};

union Vec3 {
    struct {
        float x;
        float y;
        float z;
    };
    struct {
        Vec2  xy;
        float _notused1;
    };
    struct {
        float _notused2;
        Vec2  yz;
    };
};

struct Mat4 {
    union {
        float m[16];
        struct {
            float m00, m01, m02, m03;
            float m10, m11, m12, m13;
            float m20, m21, m22, m23;
            float m30, m31, m32, m33;
        };
    };
};

Vec2 vec2(float x, float y);

Vec3  vec3(float x, float y, float z);
Vec3  vec3_sub(Vec3 v1, Vec3 v2);
float vec3_length(Vec3 v);
Vec3  vec3_norm(Vec3 v);
Vec3  vec3_cross(Vec3 v1, Vec3 v2);
float vec3_dot(Vec3 v1, Vec3 v2);

Mat4 mat4_identity(void);
Mat4 mat4_perspective(float fov, float aspect, float near, float far);
Mat4 mat4_lookat(Vec3 eye, Vec3 center, Vec3 up);

#ifdef __cplusplus
}
#endif
