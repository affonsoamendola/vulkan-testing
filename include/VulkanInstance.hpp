#pragma once

//Holds the device extensions we'll need.
const std::vector<const char*> required_device_extensions =
{

    VK_KHR_SWAPCHAIN_EXTENSION_NAME
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

#pragma once

#include <vector>
#include <vulkan/vulkan.h>

//Holds the requested validation layers.
const std::vector<const char*> validation_layers =
{

    "VK_LAYER_KHRONOS_validation"
};

//Creates the debug Utils Messenger.
VkResult create_debug_utils_messenger_EXT
(
    VkInstance instance, 
    const VkDebugUtilsMessengerCreateInfoEXT* ptr_create_info, 
    const VkAllocationCallbacks* ptr_allocator, 
    VkDebugUtilsMessengerEXT* ptr_debug_messenger
);

void destroy_debug_utils_messenger_EXT
(
    VkInstance instance, 
    VkDebugUtilsMessengerEXT debug_messenger,
    const VkAllocationCallbacks* ptr_allocator
);

