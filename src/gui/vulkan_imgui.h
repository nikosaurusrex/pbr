#pragma once

#include "nvulkan/nvulkan.h"

#include "models/models.h"

VkDescriptorPool gui_init(GLFWwindow *window, VkInstance instance, VkPhysicalDevice pdevice, Device *ldevice, U32 image_count,
                          VkRenderPass render_pass, VkCommandPool cmd_pool);
void             gui_deinit();

void gui_new_frame(void);
void gui_end_frame(VkCommandBuffer cmd_buf);

void gui_render(void);

void gui_render_materials(Materials *materials);
