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
    command_buffers_start.resize(render_target_framebuffers.size());
    command_buffers_dynamic.resize(render_target_framebuffers.size());
    command_buffers_end.resize(render_target_framebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) render_target_framebuffers.size();

    if (vkAllocateCommandBuffers(logical_device, &allocInfo, command_buffers_start.data()) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to allocate command buffers.");
    }

    if (vkAllocateCommandBuffers(logical_device, &allocInfo, command_buffers_end.data()) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to allocate command buffers.");
    }

    for (size_t i = 0; i < render_target_framebuffers.size(); i++) 
    {
        start_render_cmd(i);
        end_render_cmd(i);
    }
}

//Holds the instructions executed every loop of the renderer.
void Vulkan::start_render_cmd(uint32_t current_framebuffer)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    //Begin the GPU command sequence.
    if (vkBeginCommandBuffer(command_buffers_start[current_framebuffer], &beginInfo) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to begin recording command buffer.");
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = render_pass;
    renderPassInfo.framebuffer = render_target_framebuffers[current_framebuffer];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = render_target_image_extent;

    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;



    //Buffers the needed commands to render the 3D part.
    vkCmdBeginRenderPass(command_buffers_start[current_framebuffer], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(command_buffers_start[current_framebuffer], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
        
        VkBuffer vertex_buffers[] = {vertex_buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffers_start[current_framebuffer], 0, 1, vertex_buffers, offsets);
        vkCmdBindIndexBuffer(command_buffers_start[current_framebuffer], index_buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(command_buffers_start[current_framebuffer], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_sets[current_framebuffer], 0, nullptr);
        
        vkCmdDrawIndexed(command_buffers_start[current_framebuffer], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
   vkCmdEndRenderPass(command_buffers_start[current_framebuffer]);

    transition_image_layout_cmd(    command_buffers_start[current_framebuffer],
                                    render_target_images[current_framebuffer], 
                                    swap_chain_image_format, 
                                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    //End the GPU instructions.
    if (vkEndCommandBuffer(command_buffers_start[current_framebuffer]) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to record command buffer.");
    }
}

//Holds the instructions executed every loop of the renderer.
void Vulkan::end_render_cmd(uint32_t current_framebuffer)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    //Begin the GPU command sequence.
    if (vkBeginCommandBuffer(command_buffers_end[current_framebuffer], &beginInfo) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to begin recording command buffer.");
    }

    transition_image_layout_cmd(    command_buffers_end[current_framebuffer],
                                    render_target_images[current_framebuffer], 
                                    swap_chain_image_format, 
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    transition_image_layout_cmd(    command_buffers_end[current_framebuffer],
                                    swap_chain_images[current_framebuffer], 
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
    vkCmdBlitImage( command_buffers_end[current_framebuffer], 
                    render_target_images[current_framebuffer],
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    swap_chain_images[current_framebuffer],
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1,
                    &imageBlit,
                    VK_FILTER_NEAREST);


    transition_image_layout_cmd(    command_buffers_end[current_framebuffer],
                                    swap_chain_images[current_framebuffer], 
                                    swap_chain_image_format, 
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    //End the GPU instructions.
    if (vkEndCommandBuffer(command_buffers_end[current_framebuffer]) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to record command buffer.");
    }
}

//Holds the instructions executed every loop of the renderer.
VkCommandBuffer Vulkan::dynamic_render_cmd(uint32_t current_framebuffer)
{
    VkCommandBuffer dynamic_instructions = begin_one_time_commands();
    
    for(auto layer_vector : sprite_queue.queue)
    {
        for(auto sprite : layer_vector)
        {
            VkImageBlit image_blit = {};
            image_blit.srcSubresource = VULKAN_SUBRESOURCE_LAYER_COLOR;
            image_blit.srcOffsets[0] = {sprite->source.offset.x, 
                                        sprite->source.offset.y, 
                                        0};
            image_blit.srcOffsets[1] = {static_cast<int32_t>(sprite->source.extent.width + sprite->source.offset.x), 
                                        static_cast<int32_t>(sprite->source.extent.height + sprite->source.offset.y), 
                                        1};

            image_blit.dstSubresource = VULKAN_SUBRESOURCE_LAYER_COLOR;
            image_blit.dstOffsets[0] = {sprite->destination.offset.x, 
                                        sprite->destination.offset.y, 
                                        0};
            image_blit.dstOffsets[1] = {static_cast<int32_t>(sprite->destination.extent.width + sprite->destination.offset.x), 
                                        static_cast<int32_t>(sprite->destination.extent.height + sprite->destination.offset.y), 
                                        1};
            //Queues the actual blitting.
            vkCmdBlitImage( dynamic_instructions, 
                            sprite->ptr_texture->image,
                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                            render_target_images[current_framebuffer],
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            1,
                            &image_blit,
                            VK_FILTER_NEAREST);
        }
    }   

    //End the GPU instructions.
    if (vkEndCommandBuffer(dynamic_instructions) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to record command buffer.");
    }
    
    return dynamic_instructions;
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
    else if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(old_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
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

void Vulkan::queue_submit(  VkQueue queue,
                            VkCommandBuffer* ptr_buffer,
                            VkSemaphore wait_semaphore, 
                            VkPipelineStageFlags wait_stage,
                            VkSemaphore signal_semaphore,
                            VkFence fence)
{
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    //Tell Queue to wait for the image aquisition when it reaches the color attachment output stage.
    VkSemaphore wait_semaphores[] = {wait_semaphore};
    VkPipelineStageFlags wait_stages[] = {wait_stage};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    //Tells the queue to execture the commandbuffer previously define for the imageIndex.
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = ptr_buffer;

    //Tell GPU to signal the vk_kenderFinishedSemaphore for this framebuffer when that operation is done
    VkSemaphore signal_semaphores[] = {signal_semaphore};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

     //Sends the command buffer to the graphics queue, to be processed, sets fence when done.
    if (vkQueueSubmit(queue, 1, &submit_info, fence) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to submit draw command buffer.");
    }
}