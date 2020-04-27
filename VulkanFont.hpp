#pragma once

#include <vulkan/vulkan.h>

struct VulkanFont
{
	VulkanTexture texture;
	VkExtent2D dimensions;

	VulkanFont(	Vulkan* t_vulkan_instance,
				VkExtent2D t_dimensions,
				const char * t_file_location);
};