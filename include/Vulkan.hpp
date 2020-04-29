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
#include <algorithm>
#include <set>
#include <fstream>
#include <cstddef>
#include <array>

#include "Vector2f.hpp"
#include "Vector3f.hpp"

#include "VulkanControl.hpp"
#include "VulkanFont.hpp"
#include "VulkanInstance.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanSprite.hpp"
#include "VulkanVertex.hpp"

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

    VkSwapchainKHR        swap_chain;
    std::vector<VkImage>  swap_chain_images;
    VkFormat              swap_chain_image_format;
    VkImageLayout         swap_chain_image_layout;
    VkExtent2D            swap_chain_image_extent;

    std::vector<VkImageView>    swap_chain_image_views;

    VkDescriptorSetLayout   descriptor_set_layout;
    VkDescriptorPool        descriptor_pool;
    std::vector<VkDescriptorSet> descriptor_sets;

    VkSampler texture_sampler;

    VkPipelineLayout    pipeline_layout;
    VkPipeline          graphics_pipeline;
    VkRenderPass        render_pass;
    
    VkCommandPool       command_pool;

    std::vector<VkCommandBuffer> command_buffers_start;
    std::vector<VkCommandBuffer> command_buffers_dynamic;
    std::vector<VkCommandBuffer> command_buffers_end;

    std::vector<VkSemaphore> image_available_semaphores;
    std::vector<VkSemaphore> render_start_finished_semaphores;
    std::vector<VkSemaphore> render_dynamic_finished_semaphores;
    std::vector<VkSemaphore> render_end_finished_semaphores;
    
    std::vector<VkFence> in_flight_fences;
    std::vector<VkFence> images_in_flight;

    std::vector<Vertex> vertices = 
    {
        {{0.5,  0.5, -0.5}, {1.0f, 0.0f, 0.0f}},
        {{0.5, -0.5, -0.5}, {0.0f, 1.0f, 0.0f}},
        {{-0.5,-0.5, -0.5}, {0.0f, 0.0f, 1.0f}},
        {{-0.5, 0.5, -0.5}, {1.0f, 1.0f, 1.0f}},
        {{0.5,  0.5,  0.5}, {0.0f, 0.0f, 1.0f}},
        {{0.5, -0.5,  0.5}, {1.0f, 1.0f, 1.0f}},
        {{-0.5,-0.5,  0.5}, {1.0f, 0.0f, 0.0f}},
        {{-0.5, 0.5,  0.5}, {0.0f, 1.0f, 0.0f}}
    };

    std::vector<uint32_t> indices = 
    {
        3, 0, 1, //front
        1, 2, 3, 
        2, 1, 5, //Bot
        5, 6, 2,
        5, 4, 7, //back
        7, 6, 5,
        4, 0, 3, //top
        3, 7, 4,
        0, 4, 5, //Right
        5, 1, 0,
        6, 7, 3, //Left
        3, 2, 6
    };
        
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;

    VkBuffer index_buffer;
    VkDeviceMemory index_buffer_memory;

    std::vector<VkBuffer> uniform_buffers;
    std::vector<VkDeviceMemory> uniform_buffers_memory;

    VulkanFont* tiny_font;
    VulkanSpriteQueue sprite_queue;

    size_t current_frame = 0;

     //INSTANCE
    Vulkan();
    ~Vulkan();

    void window_init();
	void create_surface();

	void create_vulkan_instance();

	std::vector<const char*> get_required_extensions();

	void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info);
	void setup_debug_messenger();
	bool check_validation_layer_support();

	QueueFamilyIndices find_queue_families(VkPhysicalDevice device);

	void pick_physical_device();
	bool check_device_extension_support(VkPhysicalDevice device);
	bool is_device_suitable(VkPhysicalDevice device);

	void create_logical_device();

    //RENDERER
    VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
    VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes); 
    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities); 
    SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device);
    
    void create_swap_chain();
    void create_swap_chain_image_views();

    void create_render_targets();
    void create_render_target_image_views();

	void create_render_pass();

	VkShaderModule create_shader_module(const std::vector<char>& code);

	void create_graphics_pipeline();

	void create_framebuffers(); 

    void create_texture_sampler();

    void cpu_draw_frames(uint32_t current_framebuffer);
    void draw_frames();

    double get_FPS();

    //COMMAND
    void update_uniform_buffer(uint32_t current_image);

    //Vertex Buffer
    void create_vertex_buffer();

    //Index Buffer
    void create_index_buffer();
    void create_uniform_buffers();

    //Descriptor Set Layout
    void create_descriptor_set_layout();
    void create_descriptor_pool();
    void create_descriptor_sets();

    //Commands
	void create_command_pool();
	void create_render_command_buffers(); 

    void start_render_cmd(uint32_t current_framebuffer);
    VkCommandBuffer dynamic_render_cmd(uint32_t current_framebuffer);
    void end_render_cmd(uint32_t current_framebuffer);

	VkCommandBuffer begin_one_time_commands();
    void end_one_time_commands(VkCommandBuffer command_buffer);

    void exec_copy_buffer_cmd(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);


    void transition_image_layout_cmd(   VkCommandBuffer command_buffer,
                                        VkImage image, VkFormat format, 
                                        VkImageLayout old_layout,
                                        VkImageLayout new_layout);

    void exec_transition_image_layout_cmd(  VkImage image, VkFormat format, 
                                            VkImageLayout old_layout,
                                            VkImageLayout new_layout);

    void exec_copy_buffer_to_image_cmd( VkBuffer buffer, VkImage image, 
                                        uint32_t width, uint32_t height);
    
    //Buffer
    void create_buffer( VkDeviceSize size, VkBufferUsageFlags usage,
                        VkMemoryPropertyFlags properties,
                        VkBuffer& buffer, VkDeviceMemory& memory);

    //Image
    void create_vulkan_image(   uint32_t width, uint32_t height, 
                                VkFormat format, 
                                VkImageUsageFlags usage, 
                                VkImage& image, VkDeviceMemory& memory);

    VkImageView create_image_view(  VkImage image, 
                                    VkFormat format);

    //Sync
    void create_semaphore(VkSemaphore& semaphore);
    void create_fence(VkFence& fence);
    void queue_submit(  VkQueue queue,
                        VkCommandBuffer* ptr_buffer,
                        VkSemaphore wait_semaphore, 
                        VkPipelineStageFlags wait_stage,
                        VkSemaphore signal_semaphore,
                        VkFence fence);

    void create_sync_objects(); 
    void destroy_sync_objects();

    //CPU Draw Orders
    void draw_text( VulkanFont& font,
                    const char * content,
                    const VkOffset2D& offset,
                    uint32_t layer);
    void draw_char( VulkanFont& font,
                    char content,
                    const VkOffset2D& offset,
                    uint32_t layer);

    //Misc
    uint32_t find_memory_type(  VkPhysicalDevice device,
                                uint32_t type_filter, 
                                VkMemoryPropertyFlags properties);
};

#include "VulkanTexture.hpp"