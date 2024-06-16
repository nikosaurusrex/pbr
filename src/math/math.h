#pragma once

#ifndef __cplusplus
typedef struct Vector2f Vector2f;
typedef struct Vector3f Vector3f;
typedef struct Matrix4f Matrix4f;
#else
extern "C" {
#endif

struct Vector3f {
    float x;
    float y;
    float z;
};

struct Vector2f {
    float x;
    float y;
};

struct Matrix4f {
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

void mat4_identity(Matrix4f *out);
void mat4_perspective(Matrix4f *out, float fov, float aspect, float near, float far);

#ifdef __cplusplus
}
#endif
