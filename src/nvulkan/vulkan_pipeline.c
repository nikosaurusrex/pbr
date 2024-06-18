#include "nvulkan.h"

// Keep this here so we know later where we have to use it
static VkAllocationCallbacks *g_allocator = 0;

Pipeline
pipeline_create(Device *ldevice, DescriptorSet *desc_set, VkRenderPass render_pass, Shader *shaders, u32 shader_count,
                VkVertexInputBindingDescription *binding_descriptions, u32 binding_description_count,
                VkVertexInputAttributeDescription *attribute_descriptions, u32 attribute_description_count, u32 cull_mode)
{
    Pipeline p = {0};

    p.shader_stages               = malloc(shader_count * sizeof(VkPipelineShaderStageCreateInfo));
    p.shader_count                = shader_count;
    p.binding_descriptions        = binding_descriptions;
    p.binding_description_count   = binding_description_count;
    p.attribute_descriptions      = attribute_descriptions;
    p.attribute_description_count = attribute_description_count;

    for (u32 i = 0; i < shader_count; ++i) {
        FILE *f = fopen(shaders[i].path, "rb");
        if (!f) {
            fprintf(stderr, "Failed to open shader file: %s\n", shaders[i].path);
            return p;
        }

        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);
        fseek(f, 0, SEEK_SET);

        char *code = malloc(size);
        fread(code, 1, size, f);
        fclose(f);

        VkShaderModuleCreateInfo module_info = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        module_info.codeSize                 = size;
        module_info.pCode                    = (u32 *)code;

        VkShaderModule module;
        VK_CHECK(vkCreateShaderModule(ldevice->handle, &module_info, g_allocator, &module));

        VkPipelineShaderStageCreateInfo stage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        stage.stage                           = shaders[i].stage;
        stage.module                          = module;
        stage.pName                           = "main";

        p.shader_stages[i] = stage;
        free(code);
    }

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    input_assembly_state.topology                               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_state.primitiveRestartEnable                 = VK_FALSE;

    VkPipelineRasterizationStateCreateInfo rasterization_state = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterization_state.polygonMode                            = VK_POLYGON_MODE_FILL;
    rasterization_state.cullMode                               = cull_mode;
    rasterization_state.frontFace                              = VK_FRONT_FACE_CLOCKWISE;
    rasterization_state.lineWidth                              = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisample_state = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisample_state.rasterizationSamples                 = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depth_stencil_state.depthTestEnable                       = VK_TRUE;
    depth_stencil_state.depthWriteEnable                      = VK_TRUE;
    depth_stencil_state.depthCompareOp                        = VK_COMPARE_OP_LESS;

    VkPipelineViewportStateCreateInfo viewport_stage = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewport_stage.viewportCount                     = 1;
    viewport_stage.scissorCount                      = 1;

    VkDynamicState                   dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_stage    = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamic_stage.dynamicStateCount                   = 2;
    dynamic_stage.pDynamicStates                      = dynamic_states;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {0};
    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable         = VK_TRUE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.colorBlendOp        = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp        = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blend_stage = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    color_blend_stage.attachmentCount                     = 1;
    color_blend_stage.pAttachments                        = &color_blend_attachment;

    VkPipelineVertexInputStateCreateInfo vertex_input_stage = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertex_input_stage.vertexBindingDescriptionCount        = binding_description_count;
    vertex_input_stage.pVertexBindingDescriptions           = binding_descriptions;
    vertex_input_stage.vertexAttributeDescriptionCount      = attribute_description_count;
    vertex_input_stage.pVertexAttributeDescriptions         = attribute_descriptions;

    VkPipelineLayoutCreateInfo layout_info = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    layout_info.setLayoutCount             = 1;
    layout_info.pSetLayouts                = &desc_set->layout;

    VK_CHECK(vkCreatePipelineLayout(ldevice->handle, &layout_info, 0, &p.layout));

    VkGraphicsPipelineCreateInfo pipeline_info = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipeline_info.stageCount                   = shader_count;
    pipeline_info.pStages                      = p.shader_stages;
    pipeline_info.pInputAssemblyState          = &input_assembly_state;
    pipeline_info.pRasterizationState          = &rasterization_state;
    pipeline_info.pMultisampleState            = &multisample_state;
    pipeline_info.pDepthStencilState           = &depth_stencil_state;
    pipeline_info.pViewportState               = &viewport_stage;
    pipeline_info.pDynamicState                = &dynamic_stage;
    pipeline_info.pColorBlendState             = &color_blend_stage;
    pipeline_info.pVertexInputState            = &vertex_input_stage;
    pipeline_info.layout                       = p.layout;
    pipeline_info.renderPass                   = render_pass;

    // @Todo: add pipeline cache
    VK_CHECK(vkCreateGraphicsPipelines(ldevice->handle, VK_NULL_HANDLE, 1, &pipeline_info, g_allocator, &p.handle));

    return p;
}

void
pipeline_destroy(Device *ldevice, Pipeline *pipeline)
{
    for (u32 i = 0; i < pipeline->shader_count; ++i) {
        vkDestroyShaderModule(ldevice->handle, pipeline->shader_stages[i].module, g_allocator);
    }
    free(pipeline->shader_stages);

    vkDestroyPipelineLayout(ldevice->handle, pipeline->layout, g_allocator);
    vkDestroyPipeline(ldevice->handle, pipeline->handle, g_allocator);
}
