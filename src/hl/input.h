#pragma once

#include <GLFW/glfw3.h>

#ifndef __cplusplus
typedef struct Input    Input;
typedef struct MousePos MousePos;
#else
extern "C" {
#endif

struct MousePos {
    float x;
    float y;
};

struct Input {
    GLFWwindow *window;
    MousePos    mouse_pos;
    MousePos    mouse_delta_pos;
    uint8_t     locked;
};

void input_init(Input *input, GLFWwindow *window);
void input_update(Input *input);

float input_get_scroll(Input *input);

uint8_t input_is_key_down(Input *input, uint16_t key);
uint8_t input_is_button_down(Input *input, uint8_t button);

#ifdef __cplusplus
}
#endif
