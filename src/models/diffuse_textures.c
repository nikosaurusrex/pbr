#include "models.h"

#include "stb_image.h"

void
diffuse_textures_init(DiffuseTextures *textures)
{
  *textures = (DiffuseTextures){.textures = 0, .count = 0, .capacity = 1};
}

void
diffuse_textures_add(DiffuseTextures *textures, Texture texture)
{
  Assert(textures->count < max_U32);

  if (textures->count + 1 >= textures->capacity) {
    Assert(textures->capacity < max_U32 / 2);
    textures->capacity *= 2;

    textures->textures = (Texture *)realloc(textures->textures, textures->capacity * sizeof(Texture));
  }

  textures->textures[textures->count] = texture;
  textures->count++;
}

void
diffuse_textures_add_from_path(DiffuseTextures *textures, const char *path, VkPhysicalDevice pdevice, Device *ldevice,
                               VkCommandPool cmd_pool)
{
  VkSamplerCreateInfo sampler_info = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  sampler_info.minFilter           = VK_FILTER_LINEAR;
  sampler_info.magFilter           = VK_FILTER_LINEAR;
  sampler_info.mipmapMode          = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sampler_info.maxLod              = 1.0f;

  VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;

  S32 width, height, channels;
  U8 *pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
  if (!pixels) {
    log_fatal("Failed to load texture: %s", path);
  }

  Texture t = texture_from_pixels(width, height, STBI_rgb_alpha, format, pixels, sampler_info, pdevice, ldevice, cmd_pool);

  diffuse_textures_add(textures, t);
}

void
diffuse_textures_free(DiffuseTextures *textures, Device *ldevice)
{
  for (U32 i = 0; i < textures->count; ++i) {
    texture_destroy(&textures->textures[i], ldevice);
  }

  free(textures->textures);
}
