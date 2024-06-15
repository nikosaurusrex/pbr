#pragma once

#ifndef __cplusplus
typedef struct Vector2f Vector2f;
typedef struct Vector3f Vector3f;
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

#ifdef __cplusplus
}
#endif
