#pragma once

#include <vector>
#include "vulkan/vulkan.h"

//Holds the details of the swapchain
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};
