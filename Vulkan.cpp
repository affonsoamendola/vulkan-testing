#include "Vulkan.hpp"

//Initializes all of the vulkan systems
Vulkan::Vulkan()
{
    window_init();
    create_vulkan_instance();
    setup_debug_messenger();
    create_surface();
    pick_physical_device();
    create_logical_device();
    create_swap_chain();
    create_swap_chain_image_views();
    create_render_targets();
    create_render_target_image_views();
    create_render_pass();
    create_graphics_pipeline();
    create_framebuffers();
    create_command_pool();
    create_render_command_buffers();
    create_sync_objects();

    test = new VulkanTexture(this, "dopefish.png");

    sprite = new VulkanSprite(  test, 
                                {0,0,128,128}, 
                                {10,10,128,128});

    sprite_registry.register_sprite(sprite, 0);
}

//Deallocates everything that was allocated by vulkan.
Vulkan::~Vulkan()
{
    vkDeviceWaitIdle(logical_device);

    for(auto timer : swap_timers)
    {
        delete timer;
    }

    delete sprite;
    delete test;

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


uint32_t Vulkan::find_memory_type(  uint32_t type_filter, 
                                    VkMemoryPropertyFlags properties) 
{
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) 
    {
        if ((type_filter & (1 << i)) && 
            (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) 
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

