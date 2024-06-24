#pragma once

#include <GLFW/glfw3.h>

#include "base/base.h"

C_LINKAGE_BEGIN

typedef struct Input    Input;
typedef struct MousePos MousePos;

struct MousePos {
    F32 x;
    F32 y;
};

struct Input {
    GLFWwindow *window;
    MousePos    mouse_pos;
    MousePos    mouse_delta_pos;
    B8          locked;
};

void input_init(Input *input, GLFWwindow *window);
void input_update(Input *input);

F32 input_get_scroll(Input *input);

B8 input_is_key_down(Input *input, U16 key);
B8 input_is_button_down(Input *input, U8 button);

C_LINKAGE_END
