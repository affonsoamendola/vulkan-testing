#include "Vulkan.hpp"

//Creates render targets for each swapchain Image.
//TODO: Change this to use VulkanImage()
void Vulkan::create_render_targets()
{
    render_target_images.resize(swap_chain_images.size());
    render_target_device_memory.resize(swap_chain_images.size());
    render_target_image_extent = {WIDTH, HEIGHT};

    VkImage render_target;

    for(size_t i = 0; i < render_target_images.size(); i++)
    {
        VkImageCreateInfo create_info = {};

        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;
        create_info.imageType = VK_IMAGE_TYPE_2D;
        create_info.format = swap_chain_image_format;
        create_info.extent = {render_target_image_extent.width, render_target_image_extent.height, 1};
        create_info.mipLevels = 1;
        create_info.arrayLayers = 1;
        create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (vkCreateImage(logical_device, &create_info, nullptr, &render_target) != VK_SUCCESS)
        {
             throw std::runtime_error("Failed to create render target images.");
        }

        VkDeviceMemory imageMemory;

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(logical_device, render_target, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(logical_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(logical_device, render_target, imageMemory, 0);

        render_target_images[i] = render_target;
        render_target_device_memory[i] = imageMemory;
    }

    render_target_image_format = swap_chain_image_format;
}

//Creates the image views for every render_target image.
void Vulkan::create_render_target_image_views()
{
    render_target_image_views.resize(render_target_images.size());

    for(size_t i = 0; i < render_target_images.size(); i++)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = render_target_images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = render_target_image_format;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(logical_device, &createInfo, nullptr, &render_target_image_views[i]) != VK_SUCCESS) 
        {
            throw std::runtime_error("Failed to create image views.");
        }
    }
}

//TODO: Better Documentation
//Creates the Render Pass
void Vulkan::create_render_pass()
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = render_target_image_format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(logical_device, &renderPassInfo, nullptr, &render_pass) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to create render pass.");
    }
}

//Loads the bytecode of a glsl SPIR-V shader into a VkShaderModule wrapper.
VkShaderModule Vulkan::create_shader_module(const std::vector<char>& code) //THIS IS FINE
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(logical_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to create shader module.");
    }

    return shaderModule;
}

//Creates the graphics Pipeline we'll use.
void Vulkan::create_graphics_pipeline()
{
    auto vertShaderBytecode = read_file("shaders/vert.spv");
    auto fragShaderBytecode = read_file("shaders/frag.spv");

    VkShaderModule vertShaderModule = create_shader_module(vertShaderBytecode);
    VkShaderModule fragShaderModule = create_shader_module(fragShaderBytecode);

    //Adds teh vertex shader module to the pipeline
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    //Adds the fragment shader module to the pipeline
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    auto binding_description = Vertex::get_binding_description();
    auto attribute_descriptions = Vertex::get_attribute_descriptions();

    //Tells the pipeline to not care about vertex input. Since we are using hard coded vertexes.
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &binding_description; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attribute_descriptions.data(); // Optional

    //Tells the pipeline to treat the vertex list as a list of triangles.
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    //Creates the clipping area.
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) render_target_image_extent.width;
    viewport.height = (float) render_target_image_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //Defines the scissor plane
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = render_target_image_extent;

    //Creates the viewport
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    //Creates the pipeline rasterization stage.
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    //Configures multisampling.
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    //Tells the pipeline to just substitute the colors of the framebuffer by the rendered image( I think? )
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
    //Part of the above section.
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    //Creates the pipeline layout.
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1; 
    pipelineLayoutInfo.pSetLayouts = &descriptor_set_layout; 
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(logical_device, &pipelineLayoutInfo, nullptr, &pipeline_layout) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to create pipeline layout.");
    }

    //Defines the ACTUAL pipeline, using all previous defined information.
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optional
    pipelineInfo.layout = pipeline_layout;
    pipelineInfo.renderPass = render_pass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(logical_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphics_pipeline) != VK_SUCCESS)
    { 
        throw std::runtime_error("Failed to create graphics pipeline!");
    }

    //Destroys no longer needed shader modules, since they were already uploaded to the gpu.
    vkDestroyShaderModule(logical_device, fragShaderModule, nullptr); 
    vkDestroyShaderModule(logical_device, vertShaderModule, nullptr); 
}

//Creates the Framebuffer for every render_target image.
void Vulkan::create_framebuffers() 
{
    render_target_framebuffers.resize(render_target_image_views.size());

    for (size_t i = 0; i < render_target_image_views.size(); i++) 
    {
        VkImageView attachments[] = 
        {
            render_target_image_views[i]
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = render_pass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = render_target_image_extent.width;
        framebufferInfo.height = render_target_image_extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(logical_device, &framebufferInfo, nullptr, &render_target_framebuffers[i]) != VK_SUCCESS) 
        {
            throw std::runtime_error("Failed to create framebuffer.");
        }
    }
}

//Executed every frame, prepares all the data required by the gpu to draw the frames.
void Vulkan::cpu_draw_frames(uint32_t current_framebuffer)
{
    VkOffset2D off = {10, 20};
    draw_text(  *tiny_font,
                "This is Foffonso testing some Vulkan shit.",
                off,
                0);

    off = {10, 30};
    draw_text(  *tiny_font,
                "I know, amazing right?",
                off,
                0);

    off = {10, 40};
    draw_text(  *tiny_font,
                "Only took like 4000 lines of code.",
                off,
                0);

    off = {10, 60};
    draw_text(  *tiny_font,
                "And like a week...",
                off,
                0);

    off = {10, 80};
    draw_text(  *tiny_font,
                "But it kinda works.",
                off,
                0);


    update_uniform_buffer(current_framebuffer);
}

//Loop to draw every frame, ends with a request to present the image.
void Vulkan::draw_frames()
{
    //Waits for the fence for the current framebuffer to be signaled
    vkWaitForFences(logical_device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);
    
    //Gets the next image in the swapbuffer, the one we'll be rendering to.
    uint32_t imageIndex;
    vkAcquireNextImageKHR(logical_device, swap_chain, UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &imageIndex);

    // Check if a previous frame is using this image (i.e. there is its fence to wait on)
    if (images_in_flight[imageIndex] != VK_NULL_HANDLE) 
    {
        vkWaitForFences(logical_device, 1, &images_in_flight[imageIndex], VK_TRUE, UINT64_MAX);
    }

    vkFreeCommandBuffers(logical_device, command_pool, 1, &command_buffers_dynamic[imageIndex]);

    // Mark the image as now being in use by this frame
    images_in_flight[imageIndex] = in_flight_fences[current_frame];

    //Resets the current frame fence.
    vkResetFences(logical_device, 1, &in_flight_fences[current_frame]);

    cpu_draw_frames(imageIndex);

    //Queues the start section of the rendering part. Waits for the image Available semaphore
    queue_submit(   graphics_queue,
                    &command_buffers_start[imageIndex],
                    image_available_semaphores[current_frame], 
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    render_start_finished_semaphores[current_frame],
                    VK_NULL_HANDLE);

    command_buffers_dynamic[imageIndex] = dynamic_render_cmd(imageIndex);

    queue_submit(   graphics_queue,
                    &command_buffers_dynamic[imageIndex],
                    render_start_finished_semaphores[current_frame],
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    render_dynamic_finished_semaphores[current_frame],
                    VK_NULL_HANDLE);

    //Queues the end section of the rendering part, waits for the dynamic part to finish, signals the render end section
    queue_submit(   graphics_queue,
                    &command_buffers_end[imageIndex],
                    render_dynamic_finished_semaphores[current_frame],
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    render_end_finished_semaphores[current_frame],
                    in_flight_fences[current_frame]);

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    VkSemaphore ptr_wait_semaphores[] = {render_end_finished_semaphores[current_frame]};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = ptr_wait_semaphores;

    VkSwapchainKHR swapChains[] = {swap_chain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    //Queue the command to present the current frame image.
    vkQueuePresentKHR(present_queue, &presentInfo);
    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

    //std::cout << "FPS = " << get_FPS() << std::endl;

    sprite_queue.clear_queue();   
}

/*
double Vulkan::get_FPS()
{
    double average_frame_time = 0.;

    for(int i = 0; i < swap_timers.size(); i++)
    {
        average_frame_time += swap_timers[i]->delta_time();
    }

    average_frame_time /= swap_timers.size();

    return 1./average_frame_time;
}
*/
//Creates the Vertex buffer
void Vulkan::create_vertex_buffer()
{
    VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();
    
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    create_buffer(  buffer_size, 
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    staging_buffer,
                    staging_buffer_memory);

    void* data;
    vkMapMemory(logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
        memcpy(data, vertices.data(), (size_t) buffer_size);
    vkUnmapMemory(logical_device, staging_buffer_memory);

    create_buffer(  buffer_size, 
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    vertex_buffer,
                    vertex_buffer_memory);

    exec_copy_buffer_cmd(staging_buffer, vertex_buffer, buffer_size);

    vkDestroyBuffer(logical_device, staging_buffer, nullptr);
    vkFreeMemory(logical_device, staging_buffer_memory, nullptr);
}   

//Creates the Index buffer.
void Vulkan::create_index_buffer()
{
    VkDeviceSize buffer_size = sizeof(indices[0]) * indices.size();
    
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    create_buffer(  buffer_size, 
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    staging_buffer,
                    staging_buffer_memory);

    void* data;
    vkMapMemory(logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
        memcpy(data, indices.data(), (size_t) buffer_size);
    vkUnmapMemory(logical_device, staging_buffer_memory);

    create_buffer(  buffer_size, 
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    index_buffer,
                    index_buffer_memory);

    exec_copy_buffer_cmd(staging_buffer, index_buffer, buffer_size);

    vkDestroyBuffer(logical_device, staging_buffer, nullptr);
    vkFreeMemory(logical_device, staging_buffer_memory, nullptr);
}

//Creates the Index buffer.
void Vulkan::create_uniform_buffers()
{
    VkDeviceSize buffer_size = sizeof(UniformBufferObject);

    uniform_buffers.resize(swap_chain_images.size());
    uniform_buffers_memory.resize(swap_chain_images.size());

    for (size_t i = 0; i < swap_chain_images.size(); i++) 
    {
        create_buffer(  buffer_size, 
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                        uniform_buffers[i], uniform_buffers_memory[i]);
    }
}
