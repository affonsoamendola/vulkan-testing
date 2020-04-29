#include "Vulkan.hpp"
#include "vulkan/vulkan.hpp"


//INSTANCE
void Vulkan::create_vulkan_instance()  //Creates a vulkan instance.
{
    //Creates window
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
    //----
    
    if(enable_validation_layers && !check_validation_layer_support()) //Throw error if validation layers are not available.
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

    auto extensions = get_required_extensions(); //Get all extensions required to create instance.

    createInfo.enabledExtensionCount = static_cast<unsigned int>(extensions.size()); //Tell the instance creator to enable the required extensions
    createInfo.ppEnabledExtensionNames = extensions.data();
    
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo; //Struct to hold the Debug Messenger Create Informations

    if(enable_validation_layers) //If validation layers are enabled (debug build)
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
  
    if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan Instance!");
    }    

    //Create Surface for window
    if(SDL_Vulkan_CreateSurface(ptr_window, instance, &surface) != SDL_TRUE)
    {
        throw std::runtime_error("Error creating window surface.");
    }    
    //----
}

//TODO: Check the differences between instance extensions and normal extensions, is there even such a thing?
std::vector<const char*> Vulkan::get_required_extensions() //Returns all the extensions requested and required by other things like glfw.
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

    if(enable_validation_layers)   //If validation layer build we need to add an extension for it.
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

//PYSICAL DEVICE
//Gets the indices for the queue families on a physical device.
QueueFamilyIndices Vulkan::find_queue_families(VkPhysicalDevice device) 
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
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport); //Check if current queue family includes Surface Support.

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

void Vulkan::pick_physical_device() //Choose Physical device to use.
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if(deviceCount == 0)
    {
        throw std::runtime_error("Failed to find GPUs that support Vulkan.");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for(const auto& device : devices)
    {
        if(is_device_suitable(device)) //Checks if the current device is suitable.
        {
            physical_device = device;
            break;
        }
    }

    if(physical_device == VK_NULL_HANDLE) //If none found, throw error.
    {
        throw std::runtime_error("Unable to find suitable GPU.");
    }
}

//Checks if device supports all extensions we want.
bool Vulkan::check_device_extension_support(VkPhysicalDevice device)
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
bool Vulkan::is_device_suitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    QueueFamilyIndices queueIndices = find_queue_families(device);

    bool hasRequiredDeviceExtensions = check_device_extension_support(device);

    bool swapChainAdequate = false;

    if(hasRequiredDeviceExtensions)
    {
        SwapChainSupportDetails swapChainSupport = query_swap_chain_support(device); //Checks if the device supports swapchains.

        swapChainAdequate = !swapChainSupport.formats.empty() && 
                            !swapChainSupport.presentModes.empty();
    }

    return  hasRequiredDeviceExtensions &&
            swapChainAdequate && 
            deviceFeatures.samplerAnisotropy && 
            queueIndices.isComplete(); //This can mess up stuff if it found a present queue that isnt a a graphics one.
}

//Checks physical device for requested memory type
uint32_t Vulkan::find_memory_type(  VkPhysicalDevice device,
                                    uint32_t type_filter, 
                                    VkMemoryPropertyFlags properties) 
{
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(device, &mem_properties);

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


//END PHYSICAL DEVICE

//LOGICAL DEVICE
//Creates the logical device for the physical device we chose.
void Vulkan::create_logical_device()
{
    QueueFamilyIndices indices = find_queue_families(physical_device);

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

    VkPhysicalDeviceFeatures device_features = {};
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &device_features;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(required_device_extensions.size());
    createInfo.ppEnabledExtensionNames = required_device_extensions.data();

    if(enable_validation_layers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        createInfo.ppEnabledLayerNames = validation_layers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if(vkCreateDevice(physical_device, &createInfo, nullptr, &logical_device) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan Logical Device.");
    }

    vkGetDeviceQueue(logical_device, indices.graphicsFamily.value(), 0, &graphics_queue);
    vkGetDeviceQueue(logical_device, indices.presentFamily.value(), 0, &present_queue); //Gets the device queues and puts them on the designated holders.
}

//DEBUG / Validation Layers

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

//Populates the create info struct for the Debug Messenger.
void Vulkan::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info)
{   
    create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity =  //Sets message severity for the debugMessenger, disabled wont be tracked
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
      //VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType =    //Sets message types for the debug Messenger, disabled wont be tracked
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = debugCallback;
    create_info.pUserData = nullptr;
}

//Sets up the debug Messenger system
void Vulkan::setup_debug_messenger()  
{
    if(!enable_validation_layers) return; //We dont need a debug messenger if we are not in a debug build (Also we need validation layers to actually send messages to it.)
                                           //....I think.

    VkDebugUtilsMessengerCreateInfoEXT createInfo;  
    populate_debug_messenger_create_info(createInfo); //Populates the *actual* create info for the debugMessenger, the other one doesnt actually create a debug messenger.

    //Creates debug Messenger.
    if (create_debug_utils_messenger_EXT(instance, &createInfo, nullptr, &debug_messenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

//Checks if requested Validation Layers are available.
bool Vulkan::check_validation_layer_support() 
{
    unsigned int layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);

    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for(const char * layerName : validation_layers)
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

//Creates the debug Utils Messenger.
VkResult create_debug_utils_messenger_EXT
(
    VkInstance instance, 
    const VkDebugUtilsMessengerCreateInfoEXT* ptr_create_info, 
    const VkAllocationCallbacks* ptr_allocator, 
    VkDebugUtilsMessengerEXT* ptr_debug_messenger
) 
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr) 
    {
        return func(instance, ptr_create_info, ptr_allocator, ptr_debug_messenger);
    } 
    else 
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

//Destroys the debug messenger.
void destroy_debug_utils_messenger_EXT
(
    VkInstance instance, 
    VkDebugUtilsMessengerEXT debug_messenger,
    const VkAllocationCallbacks* ptr_allocator
) 
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) 
    {
        return func(instance, debug_messenger, ptr_allocator);
    } 
}


