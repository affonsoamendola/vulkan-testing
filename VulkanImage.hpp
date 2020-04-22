#pragma once

#include "vulkan/vulkan.h"

void createVulkanImage( VkPhysicalDevice physicalDevice, VkDevice device,
                        uint32_t width, uint32_t height, 
                        VkFormat format,
                        VkImage& image, VkDeviceMemory& memory);