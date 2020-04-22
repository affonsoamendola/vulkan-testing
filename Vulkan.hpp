#pragma once

#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"

#include "vulkan/vulkan.hpp"

#include "VulkanTexture.hpp"
#include "VulkanImage.hpp"

#include "Timer.hpp"

#include <vector>
#include <optional>
#include <iostream>
#include <stdexcept>

#define VULKAN_SUBRESOURCE_LAYER_COLOR {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}

//Holds the requested validation layers.
const std::vector<const char*> validation_layers =
{

    "VK_LAYER_KHRONOS_validation"
};

//Holds the queue family indices.
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return  graphicsFamily.has_value() && 
                presentFamily.has_value();
    }
};

//Holds the details of the swapchain
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanHolder
{
public:
	VulkanHolder();
	~VulkanHolder();

    void update()
    {
        drawFrames();
        std::cout << "FPS = " << getFPS() << std::endl;
    }

	const uint32_t WIDTH = 320;
    const uint32_t HEIGHT = 240;
    const uint32_t PIXEL_SCALE = 3;

    const int MAX_FRAMES_IN_FLIGHT = 3;

    #ifdef NDEBUG
    const bool vk_enableValidationLayers = false;
    #else
    const bool vk_enableValidationLayers = true;
    #endif

    SDL_Window*        ptr_window = nullptr; 
    VkInstance         vk_instance;
    VkSurfaceKHR       vk_surface;

    VkDebugUtilsMessengerEXT vk_debugMessenger;

    VkPhysicalDevice   vk_physicalDevice = VK_NULL_HANDLE;
    VkDevice           vk_logicalDevice;
    
    VkQueue            vk_graphicsQueue;
    VkQueue            vk_presentQueue;
    
    std::vector<VkImage>        vk_renderTargetImages;
    std::vector<VkDeviceMemory> vk_renderTargetDeviceMemory;
    std::vector<VkImageView>    vk_renderTargetImageViews;
    VkFormat                    vk_renderTargetImageFormat;
    VkExtent2D                  vk_renderTargetImageExtent;
    VkImageLayout               vk_renterTargetImageLayout;
    std::vector<VkFramebuffer>  vk_renderTargetFramebuffers;

    std::vector<Timer*>          vk_swapTimers;

    VkSwapchainKHR        vk_swapChain;
    std::vector<VkImage>  vk_swapChainImages;
    VkFormat              vk_swapChainImageFormat;
    VkImageLayout         vk_swapChainImageLayout;
    VkExtent2D            vk_swapChainImageExtent;

    std::vector<VkImageView>    vk_swapChainImageViews;
    
    VkPipelineLayout    vk_pipelineLayout;
    VkPipeline          vk_graphicsPipeline;
    VkRenderPass        vk_renderPass;
    
    VkCommandPool       vk_commandPool;

    std::vector<VkCommandBuffer> vk_commandBuffers;

    std::vector<VkSemaphore> vk_imageAvailableSemaphores;
    std::vector<VkSemaphore> vk_renderFinishedSemaphores;
    
    std::vector<VkFence> vk_inFlightFences;
    std::vector<VkFence> vk_imagesInFlight;

    //VulkanSpriteRegistry vk_spriteRegistry;

    size_t vk_currentFrame = 0;

    void windowInit();
	void createSurface();

	void vulkanInit();
	void createVulkanInstance();

	std::vector<const char*> getRequiredExtensions();

	void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setup_debug_messenger();
	bool check_validation_layer_support();

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	void pickPhysicalDevice();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);

	void createLogicalDevice();

    void createRenderTargets();
    void createRenderTargetImageViews();

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes); 
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities); 
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	void createSwapChain();
	void createSwapChainImageViews();

	void createRenderPass();

	VkShaderModule createShaderModule(const std::vector<char>& code);

	void createGraphicsPipeline();

	void createFramebuffers(); 

	void createCommandPool();
	void createCommandBuffers(); 
    void commandInstructions(uint32_t currentCommandBuffer);

	void createSyncObjects(); 

	void drawFrames();

    double getFPS();
    uint32_t findMemoryType(    uint32_t typeFilter, 
                                VkMemoryPropertyFlags properties);

    
	void cleanup();
};

//Callback function for the debug messenger.
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback
(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* ptr_callbackData,
    void* ptr_userData
)
{
    //Prints debug message to console
    //TODO: Add logging to file.
    std::cerr << "Validation Layer : " << ptr_callbackData->pMessage << std::endl;
    return VK_FALSE;       
}

uint32_t findMemoryType(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties);

void destroy_debug_utils_messenger_EXT
(
    VkInstance instance, 
    VkDebugUtilsMessengerEXT debug_messenger,
    const VkAllocationCallbacks* ptr_allocator
);