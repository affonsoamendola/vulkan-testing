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
	VkExtent2D 		image_extent;
	VkFormat		image_format;
	VkDeviceMemory 	device_memory;
};

//A sprite is a texture that is going to be rendered on screen.
//The texture can be a full image or can be a sprite atlas.
//Source is the source rectangle on the texture to be displayed at destination.
//If pixel perfect is set the image will only be dropped at destination.
//Without any filters applied.
struct VulkanSprite
{
	VulkanTexture* 	ptr_texture;
	VkRect2D		source;
	VkRect2D		destination;
	bool 			pixel_perfect;

	VulkanSprite(	VulkanTexture* t_ptr_texture, 
					VkRect2D t_source, 
					VkRect2D t_destination, 
					bool t_pixel_perfect=true);
};

struct VulkanSpriteRegistry
{
	int registered_layers = 0;
	std::vector<std::vector<VulkanSprite*>> registry;

	VulkanSpriteRegistry();

	void register_sprite(VulkanSprite* ptr_sprite, uint32_t layer);
};



