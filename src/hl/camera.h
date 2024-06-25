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
  F32  yaw;
  F32  pitch;
  F32  radius;
  F32  sensitivity;
};

void camera_init(Camera *c, Vec3 look_at);
void camera_resize(Camera *c, U32 width, U32 height);
void camera_update(Camera *c, Input *input, F32 delta);

C_LINKAGE_END
