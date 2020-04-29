#pragma once

class Vulkan;
//Holds a Texture definition, which is composed of an image, 
//its size and format, and its location on device memory.
//
//TODO: Uploading on demand to the GPU. (I dont think this will be 
//needed for any of my games, but you never know.)
struct VulkanTexture
{
	VulkanTexture(Vulkan* vulkan, const char* texture_file);
	~VulkanTexture();

	Vulkan* 		vulkan_instance;

	VkImage 		image;
	VkImageView 	image_view;
	VkExtent2D 		image_extent;
	VkFormat		image_format;
	VkDeviceMemory 	device_memory;
};


