#include <stdio.h>
#include <stdlib.h>

#include "imgui.h"

#include <GLFW/glfw3.h>

#include "base/base.h"

#include "gui/vulkan_imgui.h"
#include "models/models.h"
#include "nvulkan/nvulkan.h"

#include "hl/camera.h"
#include "hl/input.h"

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;
static B8                     g_show_gui  = false;

static void
check_vk_result(VkResult err)
{
  if (err == 0)
    return;
  fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
  if (err < 0)
    abort();
}

static void
glfw_error_callback(int error, const char *description)
{
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

struct Postprocess {
  DescriptorSet desc_set;
  Pipeline      pipeline;
};

Postprocess
postprocess_create(Device *ldevice, VkRenderPass render_pass, Texture *target_texture)
{
  Postprocess p = {};

  VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, 0};
  p.desc_set                           = descriptor_set_create(&binding, 1, ldevice);

  Shader shaders[] = {
      {"assets/shaders/pass.spv", VK_SHADER_STAGE_VERTEX_BIT},
      {"assets/shaders/post.spv", VK_SHADER_STAGE_FRAGMENT_BIT},
  };

  p.pipeline = pipeline_create(ldevice, &p.desc_set, render_pass, shaders, ArrayCount(shaders), 0, 0, 0, 0, VK_CULL_MODE_NONE);

  VkWriteDescriptorSet desc_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  desc_write.dstSet               = p.desc_set.handle;
  desc_write.dstBinding           = 0;
  desc_write.descriptorCount      = 1;
  desc_write.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  desc_write.pImageInfo           = &target_texture->descriptor;

  vkUpdateDescriptorSets(ldevice->handle, 1, &desc_write, 0, 0);

  return p;
}

void
postprocess_destroy(Postprocess *p, Device *ldevice)
{
  pipeline_destroy(&p->pipeline, ldevice);
  descriptor_set_destroy(&p->desc_set, ldevice);
}

struct WindowPointerInfo {
  Swapchain       *sc;
  VkCommandPool    cmd_pool;
  Image           *depth_buffer;
  Framebuffers    *frame_buffers;
  VkRenderPass     render_pass;
  PBRRenderer     *pbr_renderer;
  Postprocess     *postprocess;
  Camera          *camera;
  Input           *input;
  DiffuseTextures *diffuse_textures;
};

void
resize_callback(GLFWwindow *window, S32 width, S32 height)
{
  WindowPointerInfo *info = (WindowPointerInfo *)glfwGetWindowUserPointer(window);

  Swapchain       *sc      = info->sc;
  Device          *ldevice = info->sc->ldevice;
  VkPhysicalDevice pdevice = sc->pdevice;

  vkDeviceWaitIdle(ldevice->handle);
  vkQueueWaitIdle(ldevice->graphics_queue);

  // Recreate swapchain
  swapchain_update(info->sc, info->cmd_pool, 0);

  // Update imgui
  ImGui::GetIO().DisplaySize = ImVec2(width, height);

  // Recreate depth buffer, framebuffers, renderer and postprocess
  image_destroy(info->depth_buffer, ldevice);
  *info->depth_buffer = image_create_depth(pdevice, ldevice, sc, info->cmd_pool);

  frame_buffers_destroy(info->frame_buffers, ldevice);
  *info->frame_buffers = frame_buffers_create(sc, info->render_pass, info->depth_buffer);

  pbr_renderer_destroy(info->pbr_renderer, ldevice);
  *info->pbr_renderer =
      pbr_renderer_create(pdevice, ldevice, sc, info->cmd_pool, info->depth_buffer->format, info->diffuse_textures->count);

  VkWriteDescriptorSet desc_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  desc_write.dstSet               = info->postprocess->desc_set.handle;
  desc_write.dstBinding           = 0;
  desc_write.descriptorCount      = 1;
  desc_write.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  desc_write.pImageInfo           = &info->pbr_renderer->color_image.descriptor;

  vkUpdateDescriptorSets(ldevice->handle, 1, &desc_write, 0, 0);

  // resize camera
  camera_resize(info->camera, width, height);
}

void
key_callback(GLFWwindow *window, S32 key, S32 scancode, S32 action, S32 mods)
{
  WindowPointerInfo *info = (WindowPointerInfo *)glfwGetWindowUserPointer(window);

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    g_show_gui = !g_show_gui;

    info->input->locked = g_show_gui;
  }
}

S32
main(S32 argc, char *argv[])
{
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) {
    log_fatal("Failed to initialize GLFW!");
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  GLFWwindow *window = glfwCreateWindow(1280, 720, "raytracer", 0, 0);
  if (!window) {
    log_fatal("Failed to create window!");
  }

  B8 raw_mouse_input = glfwRawMouseMotionSupported();
  if (raw_mouse_input) {
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  }

  U32          req_extension_count;
  const char **req_extensions = glfwGetRequiredInstanceExtensions(&req_extension_count);

  const char *layers[] = {"VK_LAYER_KHRONOS_validation"};

  VkInstance instance =
      vulkan_instance_create("Raytracer", VK_API_VERSION_1_2, req_extensions, req_extension_count, layers, ArrayCount(layers));

  VkSurfaceKHR surface = surface_create(instance, window);

  const char      *device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  VkPhysicalDevice pdevice             = physical_device_find_compatible(instance, device_extensions, ArrayCount(device_extensions));
  if (!pdevice) {
    log_fatal("Failed to find compatible GPU device!");
  }

  Device ldevice = logical_device_create(surface, pdevice, device_extensions, ArrayCount(device_extensions), layers, ArrayCount(layers));

  VkCommandPool cmd_pool = command_pool_create(&ldevice);

  Swapchain swapchain = swapchain_create(2, surface, pdevice, &ldevice, cmd_pool);

  Image          depth_image         = image_create_depth(pdevice, &ldevice, &swapchain, cmd_pool);
  VkRenderPass   present_render_pass = render_pass_create_present(swapchain.format.format, depth_image.format, &ldevice);
  Framebuffers   frame_buffers       = frame_buffers_create(&swapchain, present_render_pass, &depth_image);
  CommandBuffers cmd_bufs            = command_buffers_allocate(&ldevice, cmd_pool, swapchain.image_count);

  DiffuseTextures diffuse_textures = {};
  diffuse_textures_init(&diffuse_textures);

  Materials materials = {};
  materials_init(&materials);

  Model           model      = {};
  ModelDescriptor model_desc = model_load(pdevice, &ldevice, cmd_pool, &model, &materials, &diffuse_textures, "assets/models/sphere.obj");

  PBRRenderer pbr_renderer = pbr_renderer_create(pdevice, &ldevice, &swapchain, cmd_pool, depth_image.format, diffuse_textures.count);
  Postprocess postprocess  = postprocess_create(&ldevice, present_render_pass, &pbr_renderer.color_image);

  VkDescriptorPool imgui_desc_pool = gui_init(window, instance, pdevice, &ldevice, swapchain.image_count, present_render_pass, cmd_pool);

  // after loading models when we know which materials are used
  materials_write_descriptors(&materials, pdevice, &ldevice, cmd_pool, &pbr_renderer.desc_set);

  // write diffuse textures to descriptor set
  /*
  if (diffuse_textures.count > 0) {
      VkDescriptorImageInfo *image_infos = (VkDescriptorImageInfo *)malloc(diffuse_textures.count * sizeof(VkDescriptorImageInfo));
      for (U32 i = 0; i < diffuse_textures.count; i++) {
          image_infos[i] = diffuse_textures.textures[i].descriptor;
      }

      // Write diffuse textures
      VkWriteDescriptorSet desc_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
      desc_write.dstSet               = pbr_renderer.desc_set.handle;
      desc_write.dstBinding           = 3;
      desc_write.descriptorCount      = diffuse_textures.count;
      desc_write.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      desc_write.pImageInfo           = image_infos;

      vkUpdateDescriptorSets(ldevice.handle, 1, &desc_write, 0, 0);

      free(image_infos);
  }*/

  models_write_descriptors(pdevice, &ldevice, cmd_pool, &pbr_renderer.desc_set, &model_desc, 1);

  Input input;
  input_init(&input, window);

  Camera camera;
  camera_init(&camera, vec3(0.0, 0.0, 0.0));
  camera_resize(&camera, swapchain.width, swapchain.height);

  WindowPointerInfo wp_info = {0};
  wp_info.cmd_pool          = cmd_pool;
  wp_info.sc                = &swapchain;
  wp_info.depth_buffer      = &depth_image;
  wp_info.frame_buffers     = &frame_buffers;
  wp_info.render_pass       = present_render_pass;
  wp_info.pbr_renderer      = &pbr_renderer;
  wp_info.postprocess       = &postprocess;
  wp_info.camera            = &camera;
  wp_info.input             = &input;
  wp_info.diffuse_textures  = &diffuse_textures;

  glfwSetWindowUserPointer(window, &wp_info);
  glfwSetFramebufferSizeCallback(window, resize_callback);
  glfwSetKeyCallback(window, key_callback);

  F32 last_frame_time = glfwGetTime();

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // calculate delta
    F32 current_frame_time = glfwGetTime();
    F32 delta_frame_time   = current_frame_time - last_frame_time;
    last_frame_time        = current_frame_time;

    // update input and camera
    input_update(&input);
    camera_update(&camera, &input, delta_frame_time);

    // render imgui windows
    gui_new_frame();

    if (g_show_gui) {
      gui_render_materials(&materials);
    }

    // acquiring image from swapchain and command buffer for frame
    U32             current_image = swapchain_acquire(&swapchain);
    VkCommandBuffer cmd_buf       = cmd_bufs.handles[current_image];
    command_buffer_begin(cmd_buf);

    VkClearValue clear_colors[2] = {0};
    clear_colors[0].color        = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clear_colors[1].depthStencil = {1.0f, 0};

    // Scene
    // @Todo only do on change
    materials_update_uniforms(&materials, cmd_buf);

    // @Todo only do on change (we can get that from update probably)
    GlobalUniforms uniforms = {camera.projection, camera.view, camera.position};
    pbr_renderer_update_uniforms(&pbr_renderer, cmd_buf, &uniforms);
    pbr_renderer_render(&pbr_renderer, &swapchain, cmd_buf, &model, 1, clear_colors);

    // Render UI
    {
      VkRenderPassBeginInfo ui_render_pass = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
      ui_render_pass.clearValueCount       = 2;
      ui_render_pass.pClearValues          = clear_colors;
      ui_render_pass.renderPass            = present_render_pass;
      ui_render_pass.framebuffer           = frame_buffers.handles[current_image];
      ui_render_pass.renderArea            = {{0, 0}, {swapchain.width, swapchain.height}};

      vkCmdBeginRenderPass(cmd_buf, &ui_render_pass, VK_SUBPASS_CONTENTS_INLINE);

      VkViewport viewport{0.0f, 0.0f, (F32)swapchain.width, (F32)swapchain.height, 0.0f, 1.0f};
      vkCmdSetViewport(cmd_buf, 0, 1, &viewport);

      VkRect2D scissor{{0, 0}, {swapchain.width, swapchain.height}};
      vkCmdSetScissor(cmd_buf, 0, 1, &scissor);

      vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, postprocess.pipeline.handle);
      vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, postprocess.pipeline.layout, 0, 1, &postprocess.desc_set.handle, 0,
                              0);

      vkCmdDraw(cmd_buf, 3, 1, 0, 0);

      gui_end_frame(cmd_buf);
      vkCmdEndRenderPass(cmd_buf);
    }

    // present render imaged and end frame
    command_buffer_end(cmd_buf);
    swapchain_present(&swapchain, &cmd_bufs);
  }

  vkDeviceWaitIdle(ldevice.handle);

  vkDestroyDescriptorPool(ldevice.handle, imgui_desc_pool, 0);

  model_free(&model, &ldevice);
  diffuse_textures_free(&diffuse_textures, &ldevice);
  materials_free(&materials, &ldevice);
  postprocess_destroy(&postprocess, &ldevice);
  pbr_renderer_destroy(&pbr_renderer, &ldevice);
  command_buffers_free(&cmd_bufs, &ldevice, cmd_pool);
  frame_buffers_destroy(&frame_buffers, &ldevice);
  render_pass_destroy(present_render_pass, &ldevice);
  image_destroy(&depth_image, &ldevice);
  swapchain_destroy(&swapchain);
  command_pool_destroy(cmd_pool, &ldevice);
  logical_device_destroy(&ldevice);
  surface_destroy(surface, instance);
  vulkan_instance_destroy(instance);

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
