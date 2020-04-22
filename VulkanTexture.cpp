#include "VulkanTexture.hpp"
#include "VulkanImage.hpp"

#include "SDL2/SDL_image.hpp"

void VulkanSpriteRegistry::register_sprite(	VulkanSprite* ptr_sprite, 
											uint32_t layer)
{
	if(layer >= registered_layers)
	{
		registry.resize(layer + 1);
		registered_layers = layer;
	}

	(registry[layer]).push_back(ptr_sprite);
}

VulkanTexture::VulkanTexture(	VkPhysicalDevice physical_device, VkDevice logical_device,
								const char* texture_file)
{
	VkImage texture_image;
	VkDeviceMemory device_memory;

	SDL_Surface* img_surface = IMG_Load(texture_file);

	if(img_surface == NULL)
	{
		std::cerr << "Error opening texture file: " << texture_file << " " << SDL_GetError() << std::endl;
		throw std::runtime_error("Error opening texture file.");
	}

	SDL_PixelFormat* sdl_format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
	
	SDL_Surface* converted_surface = SDL_ConvertSurface(img_surface, sdl_format, 0);

	if(converted_surface == NULL)
	{
		std::cerr << "Error converting texture file to RGBA8888: " << texture_file << " " << SDL_GetError() << std::endl;
		throw std::runtime_error("Error converting texture file.");
	}

	SDL_FreeFormat(sdl_format);
	SDL_FreeSurface(img_surface);

	createVulkanImage(	physical_device, logical_device, 
						image_width, image_height,
						format,
						texture_image, device_memory)
}