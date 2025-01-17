#include "camera.h"

#include <math.h>

static void camera_update_view(Camera *c);

void
camera_init(Camera *c, Vec3 look_at)
{
  c->yaw         = -pi32 / 2.0f;
  c->pitch       = pi32 / 4.0f;
  c->radius      = 10.0f;
  c->sensitivity = 0.01f;
  c->look_at     = look_at;

  c->projection = mat4_identity();
  c->view       = mat4_identity();
}

void
camera_resize(Camera *c, U32 width, U32 height)
{
  F32 aspect = (F32)width / (F32)height;

  c->projection = mat4_perspective(pi32 / 4.0f, aspect, 0.1f, 10000.0f);
  c->projection.m11 *= -1;

  camera_update_view(c);
}

void
camera_update(Camera *c, Input *input, F32 delta)
{
  F32 dx = input->mouse_delta_pos.x;
  F32 dy = input->mouse_delta_pos.y;

  B8 middle_click = input_is_button_down(input, GLFW_MOUSE_BUTTON_MIDDLE);
  B8 shift        = input_is_key_down(input, GLFW_KEY_LEFT_SHIFT);

  B8 moved = 0;

  F32 scroll = input_get_scroll(input);
  if (scroll != 0.0f) {
    c->radius -= scroll * 2.0;

    c->radius = Clamp(c->radius, 2.0f, 50.0f);

    moved = 1;
  }

  if (middle_click) {
    if (shift) {
      F32 move_speed = c->radius / 600.0f;

      F32 sy = sinf(c->yaw) * move_speed;
      F32 cy = cosf(c->yaw) * move_speed;

      c->look_at.x -= dx * sy;
      c->look_at.x -= dy * cy;

      c->look_at.z += dx * cy;
      c->look_at.z -= dy * sy;
    } else {
      c->yaw += dx * c->sensitivity;
      c->pitch += dy * c->sensitivity;

      c->yaw = fmod(c->yaw, 2 * pi32);

      c->pitch = Clamp(c->pitch, 0.2f, pi32 / 2.0f - 0.2f);
    }

    moved = 1;
  }

  if (moved) {
    camera_update_view(c);
  }
}

void
camera_update_view(Camera *c)
{
  F32 y = c->look_at.y + sinf(c->pitch) * c->radius;
  F32 h = cosf(c->pitch) * c->radius;

  F32 x = c->look_at.x + cosf(c->yaw) * h;
  F32 z = c->look_at.z + sinf(c->yaw) * h;

  c->view     = mat4_lookat(vec3(x, y, z), c->look_at, vec3(0, 1, 0));
  c->position = vec3(x, y, z);
}
