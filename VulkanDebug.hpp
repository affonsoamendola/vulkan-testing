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
