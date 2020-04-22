#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"

#include "vulkan/vulkan.hpp"

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

#include "Vulkan.hpp"
#include "Timer.hpp"

//Holds the requested validation layers.
const std::vector<const char*> vk_validationLayers =
{

    "VK_LAYER_KHRONOS_validation"
};

//Holds the device extensions we'll need.
const std::vector<const char*> vk_requiredDeviceExtensions=
{

    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

//Dumps all bytes from a file into a vector.
static std::vector<char> readFile(const std::string& filename) 
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) 
    {
        throw std::runtime_error("Failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    
    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}


//Creates the debug Utils Messenger.
VkResult createDebugUtilsMessengerEXT
(
    VkInstance instance, 
    const VkDebugUtilsMessengerCreateInfoEXT* ptr_createInfo, 
    const VkAllocationCallbacks* ptr_allocator, 
    VkDebugUtilsMessengerEXT* ptr_debugMessenger
) 
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr) 
    {
        return func(instance, ptr_createInfo, ptr_allocator, ptr_debugMessenger);
    } 
    else 
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

//Destroys the debug messenger.
void destroyDebugUtilsMessengerEXT
(
    VkInstance instance, 
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* ptr_Allocator
) 
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) 
    {
        return func(instance, debugMessenger, ptr_Allocator);
    } 
}

VulkanHolder::VulkanHolder()
{
    windowInit();
    vulkanInit();
}

VulkanHolder::~VulkanHolder()
{
    cleanup();  
}

void VulkanHolder::windowInit() //Inits and creates SDL Window.
{
    ptr_window = SDL_CreateWindow( "Foffonso's Vulkan Experiment",  
                                    SDL_WINDOWPOS_UNDEFINED, 
                                    SDL_WINDOWPOS_UNDEFINED, 
                                    WIDTH * PIXEL_SCALE, 
                                    HEIGHT * PIXEL_SCALE, 
                                    SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);

    if(ptr_window == nullptr)
    {
        throw std::runtime_error("Could not create SDL2 window.");
    }
}

void VulkanHolder::createSurface() //Creates a surface compatible with Vulkan for use with the window.
{
    if(SDL_Vulkan_CreateSurface(ptr_window, vk_instance, &vk_surface) != SDL_TRUE)
    {
        throw std::runtime_error("Error creating window surface.");
    }    
}

void VulkanHolder::vulkanInit()  //Initializes all of Vulkan.
{
    createVulkanInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createSwapChainImageViews();
    createRenderTargets();
    createRenderTargetImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();
}

void VulkanHolder::createVulkanInstance()  //Creates a vulkan instance.
{
    if(vk_enableValidationLayers && !checkValidationLayerSupport()) //Throw error if validation layers are not available.
    {
        throw std::runtime_error("Validation layers requested but not available.");
    }    

    VkApplicationInfo appInfo = {};  //Struct to hold Application Info 
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Triangle Test";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0; 

    VkInstanceCreateInfo createInfo = {}; //Struct to hold Instance Creation Information.
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo; //You need application Information to create instance.

    auto extensions = getRequiredExtensions(); //Get all extensions required to create instance.

    createInfo.enabledExtensionCount = static_cast<unsigned int>(extensions.size()); //Tell the instance creator to enable the required extensions
    createInfo.ppEnabledExtensionNames = extensions.data();
    
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo; //Struct to hold the Debug Messenger Create Informations

    if(vk_enableValidationLayers) //If validation layers are enabled (debug build)
    {
        createInfo.enabledLayerCount = static_cast<unsigned int>(vk_validationLayers.size()); //Configure the instance Create info struct to include the validation layers requested.
        createInfo.ppEnabledLayerNames = vk_validationLayers.data();                          //They should exist, since their existance was tested earlier.

        populateDebugMessengerCreateInfo(debugCreateInfo); //Populates the create info struct for the debug messsenger.
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo; //Adds the debug messenger crete info to the create instance create info, so that we can debug creation and destrucion of the instance.
    }
    else
    {
        createInfo.enabledLayerCount = 0;   //If validation layers are disabled. disable layers for this instance.
        createInfo.pNext = nullptr;
    }
    /* This apparently does nothing.
    uint32_t vk_extensionCount = 0;
    
    vkEnumerateInstanceExtensionProperties(nullptr, &vk_extensionCount, nullptr);

    std::vector<VkExtensionProperties> vk_extensions(vk_extensionCount);

    vkEnumerateInstanceExtensionProperties(nullptr, &vk_extensionCount, vk_extensions.data());
    */
    if(vkCreateInstance(&createInfo, nullptr, &vk_instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan Instance!");
    }    
}

//TODO: Check the differences between instance extensions and normal extensions, is there even such a thing?
std::vector<const char*> VulkanHolder::getRequiredExtensions() //Returns all the extensions requested and required by other things like glfw.
{                                                               //This specific case doesnt ask for any other extensions other than the ones required by glfw.
    uint32_t SDLExtensionCount = UINT32_MAX;
    std::vector<const char *> extensions;

    if(!SDL_Vulkan_GetInstanceExtensions(ptr_window, &SDLExtensionCount, nullptr))
    {
        throw std::runtime_error("Could not get info about how many Vulkan extension SDL requires.");
    }

    extensions.resize(SDLExtensionCount);
    
    if(!SDL_Vulkan_GetInstanceExtensions(ptr_window, &SDLExtensionCount, extensions.data()))
    {
        throw std::runtime_error("Could not get info about what SDL requires as Vulkan extensions.");
    }

    if(vk_enableValidationLayers)   //If validation layer build we need to add an extension for it.
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

//Populates the create info struct for the Debug Messenger.
void VulkanHolder::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{   
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =  //Sets message severity for the debugMessenger, disabled wont be tracked
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
      //VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =    //Sets message types for the debug Messenger, disabled wont be tracked
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;
}

void VulkanHolder::setupDebugMessenger()  //Sets up the debug Messenger system
{
    if(!vk_enableValidationLayers) return; //We dont need a debug messenger if we are not in a debug build (Also we need validation layers to actually send messages to it.)
                                           //....I think.

    VkDebugUtilsMessengerCreateInfoEXT createInfo;  
    populateDebugMessengerCreateInfo(createInfo); //Populates the *actual* create info for the debugMessenger, the other one doesnt actually create a debug messenger.

    //Creates debug Messenger.
    if (createDebugUtilsMessengerEXT(vk_instance, &createInfo, nullptr, &vk_debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

bool VulkanHolder::checkValidationLayerSupport() //Checks if requested Validation Layers are available.
{
    unsigned int layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);

    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for(const char * layerName : vk_validationLayers)
    {
        bool layerFound = false;
        for(const auto& layerProperties : availableLayers)
        {
            if(strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if(!layerFound)
        {
            return false; //If even a single layer requested is not available, return false.
        }    
    }

    return true;    
}

//Gets the indices for the queue families on a physical device.
QueueFamilyIndices VulkanHolder::findQueueFamilies(VkPhysicalDevice device) 
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int32_t i = 0;          //TODO: Add preferentially graphics and present on the same queue for performance.
    for(const auto& queueFamily : queueFamilies)
    {
        VkBool32 presentSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vk_surface, &presentSupport); //Check if current queue family includes Surface Support.

        if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)  //If queefamily is a graphics queue
        {
            indices.graphicsFamily = i;

            if(presentSupport)              //If tis specific queue is part of both graphics and present family, prefer it.
            {
                indices.presentFamily = i;
                if(indices.isComplete()) break;
            }
        }

        if(presentSupport)      //If there isnt any graphics queue that supports presenting. just find one that does and use it.
        {
            indices.presentFamily = i;
        }

        if(indices.isComplete()) break;

        i++;
    }

    return indices;
}

void VulkanHolder::pickPhysicalDevice() //Choose Physical device to use.
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vk_instance, &deviceCount, nullptr);

    if(deviceCount == 0)
    {
        throw std::runtime_error("Failed to find GPUs that support Vulkan.");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vk_instance, &deviceCount, devices.data());

    for(const auto& device : devices)
    {
        if(isDeviceSuitable(device)) //Checks if the current device is suitable.
        {
            vk_physicalDevice = device;
            break;
        }
    }

    if(vk_physicalDevice == VK_NULL_HANDLE) //If none found, throw error.
    {
        throw std::runtime_error("Unable to find suitable GPU.");
    }
}

//Checks if device supports all extensions we want.
//TODO: Should I add the glfw required extensions here as well?
bool VulkanHolder::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);

    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()); //Gets all the extensions supported by the device device.

    std::set<std::string> requiredExtensions(vk_requiredDeviceExtensions.begin(), vk_requiredDeviceExtensions.end());

    for(const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

//Check if the device is suitable for us
bool VulkanHolder::isDeviceSuitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    QueueFamilyIndices queueIndices = findQueueFamilies(device);

    bool hasRequiredDeviceExtensions = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;

    if(hasRequiredDeviceExtensions)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device); //Checks if the device supports swapchains.

        swapChainAdequate = !swapChainSupport.formats.empty() && 
                            !swapChainSupport.presentModes.empty();
    }

    return  hasRequiredDeviceExtensions &&
            swapChainAdequate &&
            queueIndices.isComplete(); //This can mess up stuff if it found a present queue that isnt a a graphics one.
}

//Creates the logical device for the physical device we chose.
void VulkanHolder::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(vk_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for(uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    //Creates the device Queues create infos.

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(vk_requiredDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = vk_requiredDeviceExtensions.data();

    if(vk_enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(vk_validationLayers.size());
        createInfo.ppEnabledLayerNames = vk_validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if(vkCreateDevice(vk_physicalDevice, &createInfo, nullptr, &vk_logicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan Logical Device.");
    }

    vkGetDeviceQueue(vk_logicalDevice, indices.graphicsFamily.value(), 0, &vk_graphicsQueue);
    vkGetDeviceQueue(vk_logicalDevice, indices.presentFamily.value(), 0, &vk_presentQueue); //Gets the device queues and puts them on the designated holders.
}

//Chooses the swapsurface format.
VkSurfaceFormatKHR VulkanHolder::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) 
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && 
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
        {
            return availableFormat;
        }
    }   

    return availableFormats[0];
}

//Chooses the swapsurface Present mode.
VkPresentModeKHR VulkanHolder::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) 
{
    for (const auto& availablePresentMode : availablePresentModes) 
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

//Chooses the swapsurface extent..
VkExtent2D VulkanHolder::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) 
{
 
    if (capabilities.currentExtent.width != UINT32_MAX) 
    {
        return capabilities.currentExtent;
    } 
    else 
    {
        VkExtent2D actualExtent = {WIDTH, HEIGHT};

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }
}

//Checks if the physical device supports the swap chain we need.
SwapChainSupportDetails VulkanHolder::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vk_surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vk_surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vk_surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vk_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vk_surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

//Crates the actual swapchain.
void VulkanHolder::createSwapChain() 
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(vk_physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};

    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vk_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(vk_physicalDevice);
    uint32_t queueFamilyIndices[] = 
    {
        indices.graphicsFamily.value(), indices.presentFamily.value()
    };

    if (indices.graphicsFamily != indices.presentFamily) 
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } 
    else 
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if(vkCreateSwapchainKHR(vk_logicalDevice, &createInfo, nullptr, &vk_swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Swapchain.");
    }

    vkGetSwapchainImagesKHR(vk_logicalDevice, vk_swapChain, &imageCount, nullptr);
    vk_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(vk_logicalDevice, vk_swapChain, &imageCount, vk_swapChainImages.data());

    vk_swapChainImageFormat = surfaceFormat.format;
    vk_swapChainImageExtent = extent;

    vk_swapTimers.resize(imageCount);

    for(int i = 0; i < imageCount; i++)
    {
        vk_swapTimers[i] = new Timer();
    }    
}

//Creates the image views for every swapchain image.
void VulkanHolder::createSwapChainImageViews()
{
    vk_swapChainImageViews.resize(vk_swapChainImages.size());

    for(size_t i = 0; i < vk_swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = vk_swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = vk_swapChainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(vk_logicalDevice, &createInfo, nullptr, &vk_swapChainImageViews[i]) != VK_SUCCESS) 
        {
            throw std::runtime_error("Failed to create image views.");
        }
    }
}

//Creates render targets for each swapchain Image.
void VulkanHolder::createRenderTargets()
{
    vk_renderTargetImages.resize(vk_swapChainImages.size());
    vk_renderTargetDeviceMemory.resize(vk_swapChainImages.size());
    vk_renderTargetImageExtent = {WIDTH, HEIGHT};

    VkImage renderTarget;

    for(size_t i = 0; i < vk_renderTargetImages.size(); i++)
    {
        VkImageCreateInfo create_info = {};

        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;
        create_info.imageType = VK_IMAGE_TYPE_2D;
        create_info.format = vk_swapChainImageFormat;
        create_info.extent = {vk_renderTargetImageExtent.width, vk_renderTargetImageExtent.height, 1};
        create_info.mipLevels = 1;
        create_info.arrayLayers = 1;
        create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (vkCreateImage(vk_logicalDevice, &create_info, nullptr, &renderTarget) != VK_SUCCESS)
        {
             throw std::runtime_error("Failed to create render target images.");
        }

        VkDeviceMemory imageMemory;

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(vk_logicalDevice, renderTarget, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(vk_physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(vk_logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(vk_logicalDevice, renderTarget, imageMemory, 0);

        vk_renderTargetImages[i] = renderTarget;
        vk_renderTargetDeviceMemory[i] = imageMemory;
    }

    vk_renderTargetImageFormat = vk_swapChainImageFormat;
}

//Creates the image views for every rendertarget image.
void VulkanHolder::createRenderTargetImageViews()
{
    vk_renderTargetImageViews.resize(vk_renderTargetImages.size());

    for(size_t i = 0; i < vk_renderTargetImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = vk_renderTargetImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = vk_renderTargetImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(vk_logicalDevice, &createInfo, nullptr, &vk_renderTargetImageViews[i]) != VK_SUCCESS) 
        {
            throw std::runtime_error("Failed to create image views.");
        }
    }
}

//TODO: Better Documentation
//Creates the Render Pass
void VulkanHolder::createRenderPass()
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = vk_renderTargetImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(vk_logicalDevice, &renderPassInfo, nullptr, &vk_renderPass) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to create render pass.");
    }
}

//Loads the bytecode of a glsl SPIR-V shader into a VkShaderModule wrapper.
VkShaderModule VulkanHolder::createShaderModule(const std::vector<char>& code) //THIS IS FINE
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(vk_logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to create shader module.");
    }

    return shaderModule;
}

//Creates the graphics Pipeline we'll use.
void VulkanHolder::createGraphicsPipeline()
{
    auto vertShaderBytecode = readFile("shaders/vert.spv");
    auto fragShaderBytecode = readFile("shaders/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderBytecode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderBytecode);

    //Adds teh vertex shader module to the pipeline
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    //Adds the fragment shader module to the pipeline
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    //Tells the pipeline to not care about vertex input. Since we are using hard coded vertexes.
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

    //Tells the pipeline to treat the vertex list as a list of triangles.
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    //Creates the clipping area.
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) vk_renderTargetImageExtent.width;
    viewport.height = (float) vk_renderTargetImageExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //Defines the scissor plane
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = vk_renderTargetImageExtent;

    //Creates the viewport
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    //Creates the pipeline rasterization stage.
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    //Configures multisampling.
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    //Tells the pipeline to just substitute the colors of the framebuffer by the rendered image( I think? )
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
    //Part of the above section.
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    //Creates the pipeline layout.
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(vk_logicalDevice, &pipelineLayoutInfo, nullptr, &vk_pipelineLayout) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to create pipeline layout.");
    }

    //Defines the ACTUAL pipeline, using all previous defined information.
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optional
    pipelineInfo.layout = vk_pipelineLayout;
    pipelineInfo.renderPass = vk_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(vk_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vk_graphicsPipeline) != VK_SUCCESS)
    { 
        throw std::runtime_error("Failed to create graphics pipeline!");
    }

    //Destroys no longer needed shader modules, since they were already uploaded to the gpu.
    vkDestroyShaderModule(vk_logicalDevice, fragShaderModule, nullptr); 
    vkDestroyShaderModule(vk_logicalDevice, vertShaderModule, nullptr); 
}

//Creates the Framebuffer for every renderTarget image.
void VulkanHolder::createFramebuffers() 
{
    vk_renderTargetFramebuffers.resize(vk_renderTargetImageViews.size());

    for (size_t i = 0; i < vk_renderTargetImageViews.size(); i++) 
    {
        VkImageView attachments[] = 
        {
            vk_renderTargetImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = vk_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = vk_renderTargetImageExtent.width;
        framebufferInfo.height = vk_renderTargetImageExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(vk_logicalDevice, &framebufferInfo, nullptr, &vk_renderTargetFramebuffers[i]) != VK_SUCCESS) 
        {
            throw std::runtime_error("Failed to create framebuffer.");
        }
    }
}

//TODO: Better Documentation
//Creates the command pool
void VulkanHolder::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(vk_physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = 0; // Optional

    if (vkCreateCommandPool(vk_logicalDevice, &poolInfo, nullptr, &vk_commandPool) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to create command pool.");
    }
}   

//Creates and allocates the command buffers for each framebuffer.
void VulkanHolder::createCommandBuffers() 
{   
    vk_commandBuffers.resize(vk_renderTargetFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vk_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) vk_commandBuffers.size();

    if (vkAllocateCommandBuffers(vk_logicalDevice, &allocInfo, vk_commandBuffers.data()) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to allocate command buffers.");
    }

    for (size_t i = 0; i < vk_commandBuffers.size(); i++) 
    {
        commandInstructions(i);
    }
}

void VulkanHolder::commandInstructions(uint32_t currentCommandBuffer)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    //beginInfo.flags = 0; // Optional
    //beginInfo.pInheritanceInfo = nullptr; // Optional

    //Begin the GPU command sequence.
    if (vkBeginCommandBuffer(vk_commandBuffers[currentCommandBuffer], &beginInfo) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to begin recording command buffer.");
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vk_renderPass;
    renderPassInfo.framebuffer = vk_renderTargetFramebuffers[currentCommandBuffer];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = vk_renderTargetImageExtent;

    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    //Buffers the needed commands to render the 3D part.
    vkCmdBeginRenderPass(vk_commandBuffers[currentCommandBuffer], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(vk_commandBuffers[currentCommandBuffer], VK_PIPELINE_BIND_POINT_GRAPHICS, vk_graphicsPipeline);
        vkCmdDraw(vk_commandBuffers[currentCommandBuffer], 3, 1, 0, 0);
    vkCmdEndRenderPass(vk_commandBuffers[currentCommandBuffer]);

    for(auto layerVector : vk_spriteRegistry.registry)
    {
        for(auto vulkanSprite : layerVector)
        {
            VkImageBlit imageBlit = {};
            imageBlit.srcSubresource = VULKAN_SUBRESOURCE_LAYER_COLOR;
            imageBlit.srcOffsets[0] = {vulkanSprite->source.offset.x, 
                                       vulkanSprite->source.offset.y, 0};
            imageBlit.srcOffsets[1] = {static_cast<int32_t>(vulkanSprite->source.extent.width), 
                                       static_cast<int32_t>(vulkanSprite->source.extent.height), 1};

            imageBlit.dstSubresource = VULKAN_SUBRESOURCE_LAYER_COLOR;
            imageBlit.dstOffsets[0] = {vulkanSprite->destination.offset.x, 
                                       vulkanSprite->destination.offset.y, 0};
            imageBlit.dstOffsets[1] = {static_cast<int32_t>(vulkanSprite->destination.extent.width), 
                                       static_cast<int32_t>(vulkanSprite->destination.extent.height), 1};
            //Queues the actual blitting.
            vkCmdBlitImage( vk_commandBuffers[currentCommandBuffer], 
                            vulkanSprite->texture.image,
                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                            vk_renderTargetImages[currentCommandBuffer],
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            1,
                            &imageBlit,
                            VK_FILTER_NEAREST);
        }
    }

    //Creates memory barrier to convert the swapChain image to the transfer destination layout.
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.image = vk_swapChainImages[currentCommandBuffer];

    vkCmdPipelineBarrier(   vk_commandBuffers[currentCommandBuffer], 
                            VK_PIPELINE_STAGE_TRANSFER_BIT, 
                            VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL,
                            0, NULL, 1, &barrier);

    VkImageBlit imageBlit = {};
    imageBlit.srcSubresource = VULKAN_SUBRESOURCE_LAYER_COLOR;
    imageBlit.srcOffsets[0] = {0, 0, 0};
    imageBlit.srcOffsets[1] = {static_cast<int32_t>(vk_renderTargetImageExtent.width), 
                               static_cast<int32_t>(vk_renderTargetImageExtent.height), 1};

    imageBlit.dstSubresource = VULKAN_SUBRESOURCE_LAYER_COLOR;
    imageBlit.dstOffsets[0] = {0, 0, 0};
    imageBlit.dstOffsets[1] = {static_cast<int32_t>(vk_swapChainImageExtent.width), 
                               static_cast<int32_t>(vk_swapChainImageExtent.height), 1};

    //Queues the actual blitting.
    vkCmdBlitImage( vk_commandBuffers[currentCommandBuffer], 
                    vk_renderTargetImages[currentCommandBuffer],
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    vk_swapChainImages[currentCommandBuffer],
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1,
                    &imageBlit,
                    VK_FILTER_NEAREST);

    //Convert the swapchain image to a presentable format.
    VkImageMemoryBarrier prePresentBarrier = {};
    prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    prePresentBarrier.pNext = nullptr;
    prePresentBarrier.srcAccessMask = 0;
    prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    prePresentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    prePresentBarrier.subresourceRange.baseMipLevel = 0;
    prePresentBarrier.subresourceRange.levelCount = 1;
    prePresentBarrier.subresourceRange.baseArrayLayer = 0;
    prePresentBarrier.subresourceRange.layerCount = 1;
    prePresentBarrier.image = vk_swapChainImages[currentCommandBuffer];

    vkCmdPipelineBarrier(   vk_commandBuffers[currentCommandBuffer], 
                            VK_PIPELINE_STAGE_TRANSFER_BIT, 
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL,
                            0, NULL, 1, &prePresentBarrier);

    //End the GPU instructions.
    if (vkEndCommandBuffer(vk_commandBuffers[currentCommandBuffer]) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to record command buffer.");
    }
}

//Creates the syncronization objects we'll use.
void VulkanHolder::createSyncObjects() 
{
    vk_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    vk_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    vk_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    vk_imagesInFlight.resize(vk_swapChainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
    {
        if (vkCreateSemaphore(vk_logicalDevice, &semaphoreInfo, nullptr, &vk_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(vk_logicalDevice, &semaphoreInfo, nullptr, &vk_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(vk_logicalDevice, &fenceInfo, nullptr, &vk_inFlightFences[i]) != VK_SUCCESS) 
        {

            throw std::runtime_error("Failed to create semaphores for a frame!");
        }
    }
}

//Loop to draw every frame.
void VulkanHolder::drawFrames()
{
    //Waits for the fence for the current framebuffer to be signaled
    vkWaitForFences(vk_logicalDevice, 1, &vk_inFlightFences[vk_currentFrame], VK_TRUE, UINT64_MAX);
    
    vk_swapTimers[vk_currentFrame]->stop_timer();

    //Gets the next image in the swapbuffer, the one we'll be rendering to.
    uint32_t imageIndex;
    vkAcquireNextImageKHR(vk_logicalDevice, vk_swapChain, UINT64_MAX, vk_imageAvailableSemaphores[vk_currentFrame], VK_NULL_HANDLE, &imageIndex);
    
    // Check if a previous frame is using this image (i.e. there is its fence to wait on)
    if (vk_imagesInFlight[imageIndex] != VK_NULL_HANDLE) 
    {
        vkWaitForFences(vk_logicalDevice, 1, &vk_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    // Mark the image as now being in use by this frame
    vk_imagesInFlight[imageIndex] = vk_inFlightFences[vk_currentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    //Tell Queue to wait for the image aquisition when it reaches the color attachment output stage.
    VkSemaphore waitSemaphores[] = {vk_imageAvailableSemaphores[vk_currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    //Tells the queue to execture the commandbuffer previously define for the imageIndex.
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vk_commandBuffers[imageIndex];

    //Tell GPU to signal the vk_kenderFinishedSemaphore for this framebuffer when that operation is done
    VkSemaphore signalSemaphores[] = {vk_renderFinishedSemaphores[vk_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    //Resets the current frame fence.
    vkResetFences(vk_logicalDevice, 1, &vk_inFlightFences[vk_currentFrame]);

    //Sends the command buffer to the graphics queue, to be processed, sets fence when done.
    if (vkQueueSubmit(vk_graphicsQueue, 1, &submitInfo,  vk_inFlightFences[vk_currentFrame]) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to submit draw command buffer.");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {vk_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    //Queue the command to present the current frame image.
    vkQueuePresentKHR(vk_presentQueue, &presentInfo);
    vk_currentFrame = (vk_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    vk_swapTimers[vk_currentFrame]->start_timer();
}

double VulkanHolder::getFPS()
{
    double average_frame_time = 0.;

    for(int i = 0; i < vk_swapTimers.size(); i++)
    {
        average_frame_time += vk_swapTimers[i]->delta_time();
    }

    average_frame_time /= vk_swapTimers.size();

    return 1./average_frame_time;
}

uint32_t findMemoryType(VkPhysicalDevice device,
                        uint32_t typeFilter, VkMemoryPropertyFlags properties) 
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
    {
        if ((typeFilter & (1 << i)) && 
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

//Deallocates everything that was allocated.
void VulkanHolder::cleanup()
{
    for(auto timer : vk_swapTimers)
    {
        delete timer;
    }

    vkDeviceWaitIdle(vk_logicalDevice);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
    {
        vkDestroySemaphore(vk_logicalDevice, vk_renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(vk_logicalDevice, vk_imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(vk_logicalDevice, vk_inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(vk_logicalDevice, vk_commandPool, nullptr);

    for (auto framebuffer : vk_renderTargetFramebuffers) 
    {
        vkDestroyFramebuffer(vk_logicalDevice, framebuffer, nullptr);
    }

    vkDestroyPipeline(vk_logicalDevice, vk_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(vk_logicalDevice, vk_pipelineLayout, nullptr);
    vkDestroyRenderPass(vk_logicalDevice, vk_renderPass, nullptr);

    for (auto renderTargetMem : vk_renderTargetDeviceMemory) 
    {
        vkFreeMemory(vk_logicalDevice, renderTargetMem, nullptr);
    }

    for (auto renderTarget : vk_renderTargetImages) 
    {
        vkDestroyImage(vk_logicalDevice, renderTarget, nullptr);
    }

    for (auto renderTargetImageView : vk_renderTargetImageViews) 
    {
        vkDestroyImageView(vk_logicalDevice, renderTargetImageView, nullptr);
    }
   
    for (auto imageView : vk_swapChainImageViews) 
    {
        vkDestroyImageView(vk_logicalDevice, imageView, nullptr);
    }
   
    vkDestroySwapchainKHR(vk_logicalDevice, vk_swapChain, nullptr);
    vkDestroyDevice(vk_logicalDevice, nullptr);

     if(vk_enableValidationLayers)
    {
        destroyDebugUtilsMessengerEXT(vk_instance, vk_debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(vk_instance, vk_surface, nullptr);
    vkDestroyInstance(vk_instance, nullptr);

    SDL_DestroyWindow(ptr_window);
    SDL_Quit();
}