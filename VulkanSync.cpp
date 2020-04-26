#include "Vulkan.hpp"

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