#include "input.h"

// @Todo: don't have this as a global
static f32 g_acc_scroll = 0;
static f32 g_last_scrol = 0;
static f32 g_scroll     = 0;

static void
scroll_callback(GLFWwindow *window, f64 xoffset, f64 yoffset)
{
    g_acc_scroll += (f32)yoffset;
}

void
input_init(Input *input, GLFWwindow *window)
{
    input->window          = window;
    input->mouse_delta_pos = (MousePos){0};
    input->locked          = 0;

    f64 xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    input->mouse_pos.x = (f32)xpos;
    input->mouse_pos.y = (f32)ypos;

    glfwSetScrollCallback(window, scroll_callback);
}

void
input_update(Input *input)
{
    g_scroll     = g_acc_scroll - g_last_scrol;
    g_last_scrol = g_acc_scroll;

    if (input->locked) {
        input->mouse_delta_pos.x = 0;
        input->mouse_delta_pos.y = 0;
        return;
    }

    f64 xpos, ypos;
    glfwGetCursorPos(input->window, &xpos, &ypos);

    f32 xposf = (f32)xpos;
    f32 yposf = (f32)ypos;

    input->mouse_delta_pos.x = xposf - input->mouse_pos.x;
    input->mouse_delta_pos.y = yposf - input->mouse_pos.y;

    input->mouse_pos.x = xposf;
    input->mouse_pos.y = yposf;
}

f32
input_get_scroll(Input *input)
{
    if (input->locked)
        return 0;
    return g_scroll;
}

b8
input_is_key_down(Input *input, u16 key)
{
    if (input->locked)
        return 0;

    return glfwGetKey(input->window, key) != GLFW_RELEASE;
}

b8
input_is_button_down(Input *input, u8 button)
{
    if (input->locked)
        return 0;
    return glfwGetMouseButton(input->window, button) != GLFW_RELEASE;
}
