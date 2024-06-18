#pragma once

#include <GLFW/glfw3.h>

#include "base/base.h"

C_LINKAGE_BEGIN

typedef struct Input    Input;
typedef struct MousePos MousePos;

struct MousePos {
    f32 x;
    f32 y;
};

struct Input {
    GLFWwindow *window;
    MousePos    mouse_pos;
    MousePos    mouse_delta_pos;
    b8          locked;
};

void input_init(Input *input, GLFWwindow *window);
void input_update(Input *input);

float input_get_scroll(Input *input);

b8 input_is_key_down(Input *input, u16 key);
b8 input_is_button_down(Input *input, u8 button);

C_LINKAGE_END
