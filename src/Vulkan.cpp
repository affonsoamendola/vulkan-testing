#include "Vulkan.hpp"

//Initializes all of the vulkan systems
Vulkan::Vulkan()
{
    create_vulkan_instance();
    setup_debug_messenger();
    pick_physical_device();
    create_logical_device();
    create_swap_chain();
    create_render_targets();
    create_render_pass();
    create_descriptor_set_layout();
    create_graphics_pipeline();
    create_framebuffers();
    create_command_pool();
    create_texture_sampler();
    create_vertex_buffer();
    create_index_buffer();
    create_uniform_buffers();
    create_descriptor_pool();
    create_descriptor_sets();
    create_render_command_buffers();
    create_sync_objects();

    VkExtent2D dim = {4, 6};
    tiny_font = new VulkanFont( this,
                                dim,
                                "data/tiny_font.png");
};

//Deallocates everything that was allocated by vulkan.
Vulkan::~Vulkan()
{
    vkDeviceWaitIdle(logical_device);

    vkDestroySampler(logical_device, texture_sampler, nullptr);

    delete tiny_font;
    destroy_sync_objects();

    vkDestroyCommandPool(logical_device, command_pool, nullptr);

    for (auto framebuffer : render_target_framebuffers) 
    {
        vkDestroyFramebuffer(logical_device, framebuffer, nullptr);
    }

    vkDestroyPipeline(logical_device, graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(logical_device, pipeline_layout, nullptr);
    vkDestroyRenderPass(logical_device, render_pass, nullptr);

    for (auto render_target_mem : render_target_device_memory) 
    {
        vkFreeMemory(logical_device, render_target_mem, nullptr);
    }

    for (auto render_target : render_target_images) 
    {
        vkDestroyImage(logical_device, render_target, nullptr);
    }

    for (auto render_target_image_view : render_target_image_views) 
    {
        vkDestroyImageView(logical_device, render_target_image_view, nullptr);
    }
   
    for (auto image_view : swap_chain_image_views) 
    {
        vkDestroyImageView(logical_device, image_view, nullptr);
    }
   
    vkDestroySwapchainKHR(logical_device, swap_chain, nullptr);

    vkDestroyDescriptorSetLayout(logical_device, descriptor_set_layout, nullptr);

    vkDestroyBuffer(logical_device, vertex_buffer, nullptr);
    vkFreeMemory(logical_device, vertex_buffer_memory, nullptr);
    
    vkDestroyBuffer(logical_device, index_buffer, nullptr);
    vkFreeMemory(logical_device, index_buffer_memory, nullptr);       

    for(size_t i = 0; i < swap_chain_images.size(); i++)
    {
        vkDestroyBuffer(logical_device, uniform_buffers[i], nullptr);
        vkFreeMemory(logical_device, uniform_buffers_memory[i], nullptr);
    }

    vkDestroyDescriptorPool(logical_device, descriptor_pool, nullptr);

    vkDestroyDevice(logical_device, nullptr);

    if(enable_validation_layers)
    {
        destroy_debug_utils_messenger_EXT(instance, debug_messenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    SDL_DestroyWindow(ptr_window);
    SDL_Quit();
}

