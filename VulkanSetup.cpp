
#include <set>

#include "vulkan/vulkan.h"
#include "Vulkan.hpp"


//Holds the device extensions we'll need.
const std::vector<const char*> required_device_extensions =
{

    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

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
    setup_debug_messenger();
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
    if(vk_enableValidationLayers && !check_validation_layer_support()) //Throw error if validation layers are not available.
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
        createInfo.enabledLayerCount = static_cast<unsigned int>(validation_layers.size()); //Configure the instance Create info struct to include the validation layers requested.
        createInfo.ppEnabledLayerNames = validation_layers.data();                          //They should exist, since their existance was tested earlier.

        populate_debug_messenger_create_info(debugCreateInfo); //Populates the create info struct for the debug messsenger.
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

    std::set<std::string> requiredExtensions(required_device_extensions.begin(), required_device_extensions.end());

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
    createInfo.enabledExtensionCount = static_cast<uint32_t>(required_device_extensions.size());
    createInfo.ppEnabledExtensionNames = required_device_extensions.data();

    if(vk_enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        createInfo.ppEnabledLayerNames = validation_layers.data();
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
        destroy_debug_utils_messenger_EXT(vk_instance, vk_debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(vk_instance, vk_surface, nullptr);
    vkDestroyInstance(vk_instance, nullptr);

    SDL_DestroyWindow(ptr_window);
    SDL_Quit();
}