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

VulkanHolder::VulkanHolder()
{
    windowInit();
    vulkanInit();
}

VulkanHolder::~VulkanHolder()
{

    cleanup();  
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

uint32_t VulkanHolder::findMemoryType(  uint32_t typeFilter, 
                                        VkMemoryPropertyFlags properties) 
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vk_physicalDevice, &memProperties);

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

