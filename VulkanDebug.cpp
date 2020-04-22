#include <vector>
#include <cstring>

#include "vulkan/vulkan.h"
#include "Vulkan.hpp"


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

//Populates the create info struct for the Debug Messenger.
void VulkanHolder::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info)
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

void VulkanHolder::setup_debug_messenger()  //Sets up the debug Messenger system
{
    if(!vk_enableValidationLayers) return; //We dont need a debug messenger if we are not in a debug build (Also we need validation layers to actually send messages to it.)
                                           //....I think.

    VkDebugUtilsMessengerCreateInfoEXT createInfo;  
    populate_debug_messenger_create_info(createInfo); //Populates the *actual* create info for the debugMessenger, the other one doesnt actually create a debug messenger.

    //Creates debug Messenger.
    if (create_debug_utils_messenger_EXT(vk_instance, &createInfo, nullptr, &vk_debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

bool VulkanHolder::check_validation_layer_support() //Checks if requested Validation Layers are available.
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