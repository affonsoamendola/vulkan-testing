#include "Vulkan.hpp"

VulkanFont::VulkanFont(	Vulkan* t_vulkan_instance,
						VkExtent2D t_dimensions,
						const char * t_file_location)
						: texture(t_vulkan_instance, t_file_location)
{
	dimensions = t_dimensions;
}