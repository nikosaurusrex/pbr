#pragma once

#include "nvulkan/nvulkan.h"

VkDescriptorPool gui_init(GLFWwindow *window, VkInstance instance, VkPhysicalDevice pdevice, Device *ldevice, uint32_t image_count,
                          VkRenderPass render_pass, VkCommandPool cmd_pool);
void             gui_deinit();

void gui_new_frame(void);
void gui_end_frame(VkCommandBuffer cmd_buf);

void gui_render(void);
