#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

const std::vector<const char*> vk_validationLayers =
{

    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> vk_requiredDeviceExtensions=
{

    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

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

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

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

class VulkanHolder
{
public:
    VulkanHolder()
    {
        windowInit();
        vulkanInit();
        mainLoop(); 
    }

    ~VulkanHolder()
    {
        cleanup();  
    }

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

    void windowInit() //Creates GLFW Window, also Inits glfw.
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //Disable API so we dont load OpenGL stuff.
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        ptr_window = glfwCreateWindow(WIDTH, HEIGHT, "Foffonso's Vulkan Experiment", nullptr, nullptr);
    }

    void createSurface() //Creates a surface compatible with Vulkan for use with the window.
    {
        if(glfwCreateWindowSurface(vk_instance, ptr_window, nullptr, &vk_surface) != VK_SUCCESS)
        {
            throw std::runtime_error("Error creating window surface.");
        }
    }

    void vulkanInit()  //Initializes all of Vulkan.
    {
        createVulkanInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createRenderPass();
        createGraphicsPipeline();
        createFramebuffers();
        createCommandPool();
        createCommandBuffers();
        createSyncObjects();
    }

    void createVulkanInstance()  //Creates a vulkan instance.
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
    std::vector<const char*> getRequiredExtensions() //Returns all the extensions requested and required by other things like glfw.
    {                                                //This specific case doesnt ask for any other extensions other than the ones required by glfw.
        uint32_t glfwExtensionCount = 0;
        const char ** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    
        if(vk_enableValidationLayers)   //If validation layer build we need to add an extension for it.
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    //Populates teh create info struct for the Debug Messenger.
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
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

    void setupDebugMessenger()  //Sets up the debug Messenger system
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

    bool checkValidationLayerSupport() //Checks if requested Validation Layers are available.
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
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) 
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
    
    void pickPhysicalDevice() //Choose Physical device to use.
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

    bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);

        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(vk_requiredDeviceExtensions.begin(), vk_requiredDeviceExtensions.end());

        for(const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    bool isDeviceSuitable(VkPhysicalDevice device)
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
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);

            swapChainAdequate = !swapChainSupport.formats.empty() && 
                                !swapChainSupport.presentModes.empty();
        }

        return  hasRequiredDeviceExtensions &&
                swapChainAdequate &&
                queueIndices.isComplete(); //This can mess up stuff if it found a present queue that isnt a a graphics one.
    }

    void createLogicalDevice()
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
        vkGetDeviceQueue(vk_logicalDevice, indices.presentFamily.value(), 0, &vk_presentQueue);
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) 
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

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) 
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

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
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

    void createSwapChain() //Ok
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
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

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
    }

    void createImageViews() //Ok
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

    void createRenderPass()
    {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = vk_swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

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

    VkShaderModule createShaderModule(const std::vector<char>& code) //THIS IS FINE
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

    void createGraphicsPipeline() //Should be ok
    {
        auto vertShaderBytecode = readFile("shaders/vert.spv");
        auto fragShaderBytecode = readFile("shaders/frag.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderBytecode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderBytecode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) vk_swapChainImageExtent.width;
        viewport.height = (float) vk_swapChainImageExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent = vk_swapChainImageExtent;

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

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

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

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

        vkDestroyShaderModule(vk_logicalDevice, fragShaderModule, nullptr); 
        vkDestroyShaderModule(vk_logicalDevice, vertShaderModule, nullptr); 
    }

    void createFramebuffers() //Should be ok
    {
        vk_swapChainFramebuffers.resize(vk_swapChainImageViews.size());

        for (size_t i = 0; i < vk_swapChainImageViews.size(); i++) 
        {
            VkImageView attachments[] = 
            {
                vk_swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = vk_renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = vk_swapChainImageExtent.width;
            framebufferInfo.height = vk_swapChainImageExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(vk_logicalDevice, &framebufferInfo, nullptr, &vk_swapChainFramebuffers[i]) != VK_SUCCESS) 
            {
                throw std::runtime_error("Failed to create framebuffer.");
            }
        }
    }

    void createCommandPool() //Should be ok
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

    void createCommandBuffers() //FINE
    {
        vk_commandBuffers.resize(vk_swapChainFramebuffers.size());

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
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            //beginInfo.flags = 0; // Optional
            //beginInfo.pInheritanceInfo = nullptr; // Optional

            if (vkBeginCommandBuffer(vk_commandBuffers[i], &beginInfo) != VK_SUCCESS) 
            {
                throw std::runtime_error("Failed to begin recording command buffer.");
            }

            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = vk_renderPass;
            renderPassInfo.framebuffer = vk_swapChainFramebuffers[i];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = vk_swapChainImageExtent;

            VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(vk_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
                vkCmdBindPipeline(vk_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vk_graphicsPipeline);
                vkCmdDraw(vk_commandBuffers[i], 3, 1, 0, 0);
            vkCmdEndRenderPass(vk_commandBuffers[i]);

            if (vkEndCommandBuffer(vk_commandBuffers[i]) != VK_SUCCESS) 
            {
                throw std::runtime_error("Failed to record command buffer.");
            }
        }
    }

    void createSyncObjects() 
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

    void mainLoop()
    {
        while(!glfwWindowShouldClose(ptr_window))
        {
            glfwPollEvents();
            drawFrames();
        }

        vkDeviceWaitIdle(vk_logicalDevice);
    }   

    void drawFrames()
    {
        vkWaitForFences(vk_logicalDevice, 1, &vk_inFlightFences[vk_currentFrame], VK_TRUE, UINT64_MAX);
        
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

        VkSemaphore waitSemaphores[] = {vk_imageAvailableSemaphores[vk_currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &vk_commandBuffers[imageIndex];

        VkSemaphore signalSemaphores[] = {vk_renderFinishedSemaphores[vk_currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        
        vkResetFences(vk_logicalDevice, 1, &vk_inFlightFences[vk_currentFrame]);

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

        vkQueuePresentKHR(vk_presentQueue, &presentInfo);
        vk_currentFrame = (vk_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void cleanup()
    {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            vkDestroySemaphore(vk_logicalDevice, vk_renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(vk_logicalDevice, vk_imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(vk_logicalDevice, vk_inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(vk_logicalDevice, vk_commandPool, nullptr);

        for (auto framebuffer : vk_swapChainFramebuffers) 
        {
            vkDestroyFramebuffer(vk_logicalDevice, framebuffer, nullptr);
        }

        vkDestroyPipeline(vk_logicalDevice, vk_graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(vk_logicalDevice, vk_pipelineLayout, nullptr);
        vkDestroyRenderPass(vk_logicalDevice, vk_renderPass, nullptr);

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

        glfwDestroyWindow(ptr_window);

        glfwTerminate();
    }
};

