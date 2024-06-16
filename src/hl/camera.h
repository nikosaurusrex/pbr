#pragma once

#include <stdint.h>

#include <GLFW/glfw3.h>

#include "hl/input.h"
#include "math/math.h"

#ifndef __cplusplus
typedef struct Camera Camera;
#else
extern "C" {
#endif

struct Camera {
    Mat4  projection;
    Mat4  view;
    Vec3  look_at;
    float yaw;
    float pitch;
    float radius;
    float sensitivity;
};

void camera_init(Camera *c, Vec3 look_at);
void camera_resize(Camera *c, uint32_t width, uint32_t height);
void camera_update(Camera *c, Input *input, float delta);

#ifdef __cplusplus
}
#endif
