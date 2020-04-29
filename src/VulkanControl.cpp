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

void Vulkan::create_buffer(     VkDeviceSize size, VkBufferUsageFlags usage,
                                VkMemoryPropertyFlags properties,
                                VkBuffer& buffer, VkDeviceMemory& memory)
{
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(logical_device, &buffer_info, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create buffer!");
    }

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(logical_device, buffer, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = find_memory_type(physical_device, mem_requirements.memoryTypeBits, properties);

    if(vkAllocateMemory(logical_device, &alloc_info, nullptr, &memory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate Buffer memory!");
    }

    vkBindBufferMemory(logical_device, buffer, memory, 0);
}

//CREATES INTERACTION BUFFERS
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

//END

//STANDARD RENDER COMMANDS.
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

//ONE-TIME COMMANDS
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

//COMMANDS
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

//SYNC
void Vulkan::create_semaphore(VkSemaphore& semaphore)
{
    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if(vkCreateSemaphore(logical_device, &semaphore_info, nullptr, &semaphore) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to create semaphore for a frame!");
    }
}

void Vulkan::create_fence(VkFence& fence)
{
    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    if(vkCreateFence(logical_device, &fence_info, nullptr, &fence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create fence for a frame!");
    }
}
//Creates the syncronization objects we'll use.
void Vulkan::create_sync_objects() 
{
    image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    render_start_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    render_dynamic_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    render_end_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);

    in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
    images_in_flight.resize(swap_chain_images.size(), VK_NULL_HANDLE);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
    {
        create_semaphore(image_available_semaphores[i]);
        create_semaphore(render_start_finished_semaphores[i]);
        create_semaphore(render_dynamic_finished_semaphores[i]);
        create_semaphore(render_end_finished_semaphores[i]);

        create_fence(in_flight_fences[i]);
    }
}

void Vulkan::destroy_sync_objects()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
    {
        vkDestroySemaphore(logical_device, render_start_finished_semaphores[i], nullptr);
        vkDestroySemaphore(logical_device, render_dynamic_finished_semaphores[i], nullptr);
        vkDestroySemaphore(logical_device, render_end_finished_semaphores[i], nullptr);
        vkDestroySemaphore(logical_device, image_available_semaphores[i], nullptr);
        vkDestroyFence(logical_device, in_flight_fences[i], nullptr);
    }
}

void Vulkan::create_descriptor_set_layout()
{
    VkDescriptorSetLayoutBinding ubo_layout_binding = {};

    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding sampler_layout_binding{};
    sampler_layout_binding.binding = 1;
    sampler_layout_binding.descriptorCount = 1;
    sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.pImmutableSamplers = nullptr;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {ubo_layout_binding, sampler_layout_binding};

    VkDescriptorSetLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
    layout_info.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(logical_device, &layout_info, nullptr, &descriptor_set_layout) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to create descriptor set layout.");
    }
}

void Vulkan::create_descriptor_pool()
{
    std::array<VkDescriptorPoolSize, 2> pool_sizes = {};
    pool_size[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size[0].descriptorCount = static_cast<uint32_t>(swap_chain_images.size());
    pool_size[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_size[1].descriptorCount = static_cast<uint32_t>(swap_chain_images.size());


    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes);
    pool_info.pPoolSizes = pool_sizes.data();
    pool_info.maxSets = static_cast<uint32_t>(swap_chain_images.size());

    if (vkCreateDescriptorPool(logical_device, &pool_info, nullptr, &descriptor_pool) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to create descriptor pool.");
    }
}

void Vulkan::create_descriptor_sets()
{
    std::vector<VkDescriptorSetLayout> layouts(swap_chain_images.size(), descriptor_set_layout);
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool;
    alloc_info.descriptorSetCount = static_cast<uint32_t>(swap_chain_images.size());
    alloc_info.pSetLayouts = layouts.data();

    descriptor_sets.resize(swap_chain_images.size());

    if (vkAllocateDescriptorSets(logical_device, &alloc_info, descriptor_sets.data()) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to allocate descriptor sets.");
    }

    for (size_t i = 0; i < swap_chain_images.size(); i++) 
    {
        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = uniform_buffers[i];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo image_info = {};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = textureImageView;
        image_info.sampler = textureSampler;

        std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = descriptor_sets[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &bufferInfo;

        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = descriptor_sets[i];
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets( logical_device,  
                                static_cast<uint32_t>(descriptor_writes.size()), 
                                descriptor_writes.data(), 
                                0, nullptr);
    }
}

void Vulkan::draw_text( VulkanFont& font,
                        const char * content,
                        const VkOffset2D& offset,
                        uint32_t layer)
{
    VkOffset2D current_offset;
    current_offset.y = offset.y;
    for(size_t i = 0; content[i] != '\0'; i++)
    {
        current_offset.x = offset.x + i*font.dimensions.width;

        draw_char(font, content[i], current_offset, layer);
    }
}

void Vulkan::draw_char( VulkanFont& font,
                        char content,
                        const VkOffset2D& offset,
                        uint32_t layer)
{
    char printable = content - 0x20;

    VkRect2D src =  {   (printable%0x20)*(int32_t)font.dimensions.width, 
                        (printable/0x20)*(int32_t)font.dimensions.height, 
                        font.dimensions.width, 
                        font.dimensions.height};
    VkRect2D dst = {    offset.x, offset.y, 
                        font.dimensions.width, font.dimensions.height};

    VulkanSprite* sprite = new VulkanSprite(&font.texture, 
                                            src, 
                                            dst);
    sprite_queue.queue_sprite(sprite, layer);
}

void Vulkan::update_uniform_buffer(uint32_t current_image)
{
    UniformBufferObject ubo = {};
    ubo.model = m4f_rotate(m4f_identity(), Timer::time() * 0.5f, Vector3f(0.8f, 0.0f, 0.5f));
    ubo.view  = m4f_translate(Vector3f(0.0f, 0.0f, 3.0f));
    ubo.proj  = m4f_perspective(radians(45.0f), (float) WIDTH / (float) HEIGHT, 0.1f, 10.0f);

    void* data;
    vkMapMemory(logical_device, uniform_buffers_memory[current_image], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(logical_device, uniform_buffers_memory[current_image]);
}
