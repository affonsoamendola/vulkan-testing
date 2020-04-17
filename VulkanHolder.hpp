#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <optional>
#include <iostream>
#include <stdexcept>

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
private:
	const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    const int MAX_FRAMES_IN_FLIGHT = 2;

    #ifdef NDEBUG
    const bool vk_enableValidationLayers = false;
    #else
    const bool vk_enableValidationLayers = true;
    #endif

    GLFWwindow*        ptr_window;
    VkInstance         vk_instance;
    VkSurfaceKHR       vk_surface;

    VkDebugUtilsMessengerEXT vk_debugMessenger;

    VkPhysicalDevice   vk_physicalDevice = VK_NULL_HANDLE;
    VkDevice           vk_logicalDevice;
    
    VkQueue            vk_graphicsQueue;
    VkQueue            vk_presentQueue;
    
    VkSwapchainKHR        vk_swapChain;
    std::vector<VkImage>  vk_swapChainImages;
    VkFormat              vk_swapChainImageFormat;
    VkExtent2D            vk_swapChainImageExtent;

    std::vector<VkImageView>    vk_swapChainImageViews;
    std::vector<VkFramebuffer>  vk_swapChainFramebuffers;

    VkPipelineLayout    vk_pipelineLayout;
    VkPipeline          vk_graphicsPipeline;
    VkRenderPass        vk_renderPass;
    
    VkCommandPool       vk_commandPool;

    std::vector<VkCommandBuffer> vk_commandBuffers;

    std::vector<VkSemaphore> vk_imageAvailableSemaphores;
    std::vector<VkSemaphore> vk_renderFinishedSemaphores;
    
    std::vector<VkFence> vk_inFlightFences;
    std::vector<VkFence> vk_imagesInFlight;

    size_t vk_currentFrame = 0;

    void windowInit();
	void createSurface();
	void vulkanInit();
	void createVulkanInstance();
	std::vector<const char*> getRequiredExtensions();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	bool checkValidationLayerSupport();
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	void pickPhysicalDevice();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);
	void createLogicalDevice();
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes); 
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities); 
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createGraphicsPipeline();
	void createFramebuffers(); 
	void createCommandPool();
	void createCommandBuffers(); 
	void createSyncObjects(); 
	void mainLoop();
	void drawFrames();
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