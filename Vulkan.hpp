#pragma once

#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"
#include "SDL2/SDL_image.h"

#include "vulkan/vulkan.hpp"

#include <vector>
#include <optional>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <optional>
#include <algorithm>
#include <set>
#include <fstream>
#include <cstddef>

#include "VulkanDebug.hpp"
#include "VulkanSetup.hpp"
#include "VulkanSwap.hpp"

#include "Timer.hpp"
#include "Util.hpp"

#define VULKAN_SUBRESOURCE_LAYER_COLOR {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}

class Vulkan
{
public:
	const uint32_t WIDTH = 320;
    const uint32_t HEIGHT = 240;
    const uint32_t PIXEL_SCALE = 3;

    const int MAX_FRAMES_IN_FLIGHT = 3;

    #ifdef NDEBUG
    const bool enable_validation_layers = false;
    #else
    const bool enable_validation_layers = true;
    #endif

    SDL_Window*        ptr_window = nullptr; 
    VkInstance         instance;
    VkSurfaceKHR       surface;

    VkDebugUtilsMessengerEXT debug_messenger;

    VkPhysicalDevice   physical_device = VK_NULL_HANDLE;
    VkDevice           logical_device;
    
    VkQueue            graphics_queue;
    VkQueue            present_queue;
    
    std::vector<VkImage>        render_target_images;
    std::vector<VkDeviceMemory> render_target_device_memory;
    std::vector<VkImageView>    render_target_image_views;
    VkFormat                    render_target_image_format;
    VkExtent2D                  render_target_image_extent;
    VkImageLayout               renter_target_image_layout;
    std::vector<VkFramebuffer>  render_target_framebuffers;

    std::vector<Timer*>          swap_timers;

    VkSwapchainKHR        swap_chain;
    std::vector<VkImage>  swap_chain_images;
    VkFormat              swap_chain_image_format;
    VkImageLayout         swap_chain_image_layout;
    VkExtent2D            swap_chain_image_extent;

    std::vector<VkImageView>    swap_chain_image_views;
    
    VkPipelineLayout    pipeline_layout;
    VkPipeline          graphics_pipeline;
    VkRenderPass        render_pass;
    
    VkCommandPool       command_pool;

    std::vector<VkCommandBuffer> command_buffers;

    std::vector<VkSemaphore> image_available_semaphores;
    std::vector<VkSemaphore> render_finished_semaphores;
    
    std::vector<VkFence> in_flight_fences;
    std::vector<VkFence> images_in_flight;

    //VulkanSpriteRegistry spriteRegistry;

    size_t current_frame = 0;

     //Init
    Vulkan();
    ~Vulkan();

    void window_init();
	void create_surface();

	void create_vulkan_instance();

	std::vector<const char*> get_required_extensions();

    //Debug
	void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info);
	void setup_debug_messenger();
	bool check_validation_layer_support();

    //Setup
	QueueFamilyIndices find_queue_families(VkPhysicalDevice device);

	void pick_physical_device();
	bool check_device_extension_support(VkPhysicalDevice device);
	bool is_device_suitable(VkPhysicalDevice device);

	void create_logical_device();

    //Swapchain
    VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
    VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes); 
    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities); 
    SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device);
    
    void create_swap_chain();
    void create_swap_chain_image_views();


    //Graphics Pipeline
    void create_render_targets();
    void create_render_target_image_views();

	void create_render_pass();

	VkShaderModule create_shader_module(const std::vector<char>& code);

	void create_graphics_pipeline();

	void create_framebuffers(); 

    void draw_frames();

    double get_FPS();

    //Commands

	void create_command_pool();
	void create_render_command_buffers(); 
	void create_sync_objects(); 

    void render_command_instructions(uint32_t current_command_buffer);

	VkCommandBuffer begin_one_time_commands();
    void end_one_time_commands(VkCommandBuffer command_buffer);

    void cmd_copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);

    //Vulkan Image
    void create_vulkan_image(   uint32_t width, uint32_t height, 
                                VkFormat format,
                                VkImage& image, VkDeviceMemory& memory);

    //Misc
    uint32_t find_memory_type(  uint32_t type_filter, 
                                VkMemoryPropertyFlags properties);
};

#include "VulkanTexture.hpp"