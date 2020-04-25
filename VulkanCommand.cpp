#include "Vulkan.hpp"

//TODO: Better Documentation
//Creates the command pool
void Vulkan::create_command_pool()
{
    QueueFamilyIndices queueFamilyIndices = find_queue_families(physical_device);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = 0; // Optional

    if (vkCreateCommandPool(logical_device, &poolInfo, nullptr, &command_pool) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to create command pool.");
    }
}   

//Creates and allocates the command buffers for each framebuffer.
void Vulkan::create_render_command_buffers() 
{   
    command_buffers.resize(render_target_framebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) command_buffers.size();

    if (vkAllocateCommandBuffers(logical_device, &allocInfo, command_buffers.data()) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to allocate command buffers.");
    }

    for (size_t i = 0; i < command_buffers.size(); i++) 
    {
        render_command_instructions(i);
    }
}

//Holds the instructions executed every loop of the renderer.
void Vulkan::render_command_instructions(uint32_t currentCommandBuffer)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    //Begin the GPU command sequence.
    if (vkBeginCommandBuffer(command_buffers[currentCommandBuffer], &beginInfo) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to begin recording command buffer.");
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = render_pass;
    renderPassInfo.framebuffer = render_target_framebuffers[currentCommandBuffer];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = render_target_image_extent;

    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    //Buffers the needed commands to render the 3D part.
    vkCmdBeginRenderPass(command_buffers[currentCommandBuffer], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(command_buffers[currentCommandBuffer], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
        vkCmdDraw(command_buffers[currentCommandBuffer], 3, 1, 0, 0);
    vkCmdEndRenderPass(command_buffers[currentCommandBuffer]);
    
    for(auto layer_vector : sprite_registry.registry)
    {
        for(auto sprite : layer_vector)
        {
            VkImageBlit image_blit = {};
            imageBlit.srcSubresource = VULKAN_SUBRESOURCE_LAYER_COLOR;
            imageBlit.srcOffsets[0] = {sprite->source.offset.x, 
                                       sprite->source.offset.y, 0};
            imageBlit.srcOffsets[1] = {static_cast<int32_t>(sprite->source.extent.width), 
                                       static_cast<int32_t>(sprite->source.extent.height), 1};

            imageBlit.dstSubresource = VULKAN_SUBRESOURCE_LAYER_COLOR;
            imageBlit.dstOffsets[0] = {sprite->destination.offset.x, 
                                       sprite->destination.offset.y, 0};
            imageBlit.dstOffsets[1] = {static_cast<int32_t>(sprite->destination.extent.width), 
                                       static_cast<int32_t>(sprite->destination.extent.height), 1};
            //Queues the actual blitting.
            vkCmdBlitImage( command_buffers[currentCommandBuffer], 
                            sprite->texture.image,
                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                            render_target_images[currentCommandBuffer],
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            1,
                            &imageBlit,
                            VK_FILTER_NEAREST);
        }
    }  
    
    transition_image_layout_cmd(    command_buffers[currentCommandBuffer],
                                    swap_chain_images[currentCommandBuffer], 
                                    swap_chain_image_format, 
                                    VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);


    VkImageBlit imageBlit = {};
    imageBlit.srcSubresource = VULKAN_SUBRESOURCE_LAYER_COLOR;
    imageBlit.srcOffsets[0] = {0, 0, 0};
    imageBlit.srcOffsets[1] = {static_cast<int32_t>(render_target_image_extent.width), 
                               static_cast<int32_t>(render_target_image_extent.height), 1};

    imageBlit.dstSubresource = VULKAN_SUBRESOURCE_LAYER_COLOR;
    imageBlit.dstOffsets[0] = {0, 0, 0};
    imageBlit.dstOffsets[1] = {static_cast<int32_t>(swap_chain_image_extent.width), 
                               static_cast<int32_t>(swap_chain_image_extent.height), 1};

    //Queues the actual blitting.
    vkCmdBlitImage( command_buffers[currentCommandBuffer], 
                    render_target_images[currentCommandBuffer],
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    swap_chain_images[currentCommandBuffer],
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1,
                    &imageBlit,
                    VK_FILTER_NEAREST);


    transition_image_layout_cmd(    command_buffers[currentCommandBuffer],
                                    swap_chain_images[currentCommandBuffer], 
                                    swap_chain_image_format, 
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    //End the GPU instructions.
    if (vkEndCommandBuffer(command_buffers[currentCommandBuffer]) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to record command buffer.");
    }
}

//Begins and allocates a command buffer for one time commands.
VkCommandBuffer Vulkan::begin_one_time_commands()
{
    VkCommandBufferAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandPool = command_pool;
    allocate_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(logical_device, &allocate_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);
    return command_buffer;
}

//Ends, submits and deallocates a command buffer for one time commands.
void Vulkan::end_one_time_commands(VkCommandBuffer command_buffer)
{
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(logical_device, command_pool, 1, &command_buffer);
}

//Executes a copy buffer command on the GPU.
void Vulkan::exec_copy_buffer_cmd(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size)
{
    VkCommandBuffer command_buffer = begin_one_time_commands();

    VkBufferCopy copy_region = {};
    copy_region.size = size;

    vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

    end_one_time_commands(command_buffer);
}

//Executes a copy buffer to image command.
void Vulkan::exec_copy_buffer_to_image_cmd( VkBuffer buffer, VkImage image, 
                                            uint32_t width, uint32_t height)
{
    VkCommandBuffer command_buffer = begin_one_time_commands();

    VkBufferImageCopy region = {};

    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage( command_buffer, 
                            buffer, 
                            image, 
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            1,
                            &region);

    end_one_time_commands(command_buffer);
}


//Issues a transition image layout cmd to the command buffer.
void Vulkan::transition_image_layout_cmd(   VkCommandBuffer command_buffer,
                                            VkImage image, VkFormat format, 
                                            VkImageLayout old_layout,
                                            VkImageLayout new_layout)
{
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;

    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if(old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
    else
    {

        throw std::invalid_argument("Tried to transition unsupported layer types.");
    }

    vkCmdPipelineBarrier(   command_buffer,
                            source_stage, destination_stage,
                            0,
                            0, nullptr,
                            0, nullptr,
                            1, &barrier);
}

//Executes a transition image layout cmd to the command buffer.
void Vulkan::exec_transition_image_layout_cmd(  VkImage image, VkFormat format, 
                                                VkImageLayout old_layout,
                                                VkImageLayout new_layout)
{
    VkCommandBuffer command_buffer = begin_one_time_commands();

    transition_image_layout_cmd(command_buffer, image, format, old_layout, new_layout);

    end_one_time_commands(command_buffer);
}

//Creates the syncronization objects we'll use.
void Vulkan::create_sync_objects() 
{
    image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
    images_in_flight.resize(swap_chain_images.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
    {
        if (vkCreateSemaphore(logical_device, &semaphoreInfo, nullptr, &image_available_semaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(logical_device, &semaphoreInfo, nullptr, &render_finished_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(logical_device, &fenceInfo, nullptr, &in_flight_fences[i]) != VK_SUCCESS) 
        {

            throw std::runtime_error("Failed to create semaphores for a frame!");
        }
    }
}
