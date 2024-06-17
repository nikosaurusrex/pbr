#include "camera.h"

#include <math.h>

#define PI 3.141592653589f

static void camera_update_view(Camera *c);

void
camera_init(Camera *c, Vec3 look_at)
{
    c->yaw         = -PI / 2.0f;
    c->pitch       = PI / 4.0f;
    c->radius      = 10.0f;
    c->sensitivity = 0.01f;
    c->look_at     = look_at;

    c->projection = mat4_identity();
    c->view = mat4_identity();
}

void
camera_resize(Camera *c, uint32_t width, uint32_t height)
{
    float aspect = (float)width / (float)height;

    c->projection = mat4_perspective(PI / 4.0f, aspect, 0.1f, 100.0f);
    c->projection.m11 *= -1;

    camera_update_view(c);
}

void
camera_update(Camera *c, Input *input, float delta)
{
    float dx = input->mouse_delta_pos.x;
    float dy = input->mouse_delta_pos.y;

    uint8_t right_click = input_is_button_down(input, GLFW_MOUSE_BUTTON_RIGHT);
    uint8_t shift       = input_is_key_down(input, GLFW_KEY_LEFT_SHIFT);

    uint8_t moved = 0;

    float scroll = input_get_scroll(input);
    if (scroll != 0.0f) {
        c->radius -= scroll;

        if (c->radius < 2)
            c->radius = 2;
        if (c->radius > 30)
            c->radius = 30;

        moved = 1;
    }

    if (right_click) {
        if (shift) {
            float move_speed = c->radius / 600.0f;

            float sy = sin(c->yaw) * move_speed;
            float cy = cos(c->yaw) * move_speed;

            c->look_at.x -= dx * sy;
            c->look_at.x -= dy * cy;

            c->look_at.z += dx * cy;
            c->look_at.z -= dy * sy;
        } else {
            c->yaw += dx * c->sensitivity;
            c->pitch += dy * c->sensitivity;

            c->yaw = fmod(c->yaw, 2 * PI);
            if (c->pitch < 0.2f)
                c->pitch = 0.2f;
            if (c->pitch > PI / 2.0f - 0.2f)
                c->pitch = PI / 2.0f - 0.2f;
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
    float y = c->look_at.y + sin(c->pitch) * c->radius;
    float h = cos(c->pitch) * c->radius;

    float x = c->look_at.x + cos(c->yaw) * h;
    float z = c->look_at.z + sin(c->yaw) * h;

    c->view = mat4_lookat(vec3(x, y, z), c->look_at, vec3(0, 1, 0));
}
