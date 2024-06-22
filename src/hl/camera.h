#pragma once

#include <GLFW/glfw3.h>

#include "base/base.h"

#include "hl/input.h"
#include "math/math.h"

C_LINKAGE_BEGIN

typedef struct Camera Camera;

struct Camera {
    Mat4 projection;
    Mat4 view;
    Vec3 position;
    Vec3 look_at;
    f32  yaw;
    f32  pitch;
    f32  radius;
    f32  sensitivity;
};

void camera_init(Camera *c, Vec3 look_at);
void camera_resize(Camera *c, u32 width, u32 height);
void camera_update(Camera *c, Input *input, f32 delta);

C_LINKAGE_END
