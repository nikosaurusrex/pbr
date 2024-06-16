#include "input.h"

// @Todo: don't have this as a global
static float g_acc_scroll = 0;
static float g_last_scrol = 0;
static float g_scroll     = 0;

static void
scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    g_acc_scroll += (float)yoffset;
}

void
input_init(Input *input, GLFWwindow *window)
{
    input->window          = window;
    input->mouse_delta_pos = (MousePos){0};
    input->locked          = 0;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    input->mouse_pos.x = (float)xpos;
    input->mouse_pos.y = (float)ypos;

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

    double xpos, ypos;
    glfwGetCursorPos(input->window, &xpos, &ypos);

    float xposf = (float)xpos;
    float yposf = (float)ypos;

    input->mouse_delta_pos.x = xposf - input->mouse_pos.x;
    input->mouse_delta_pos.y = yposf - input->mouse_pos.y;

    input->mouse_pos.x = xposf;
    input->mouse_pos.y = yposf;
}

float
input_get_scroll(Input *input)
{
    if (input->locked)
        return 0;
    return g_scroll;
}

uint8_t
input_is_key_down(Input *input, uint16_t key)
{
    if (input->locked)
        return 0;

    return glfwGetKey(input->window, key) != GLFW_RELEASE;
}

uint8_t
input_is_button_down(Input *input, uint8_t button)
{
    if (input->locked)
        return 0;
    return glfwGetMouseButton(input->window, button) != GLFW_RELEASE;
}
