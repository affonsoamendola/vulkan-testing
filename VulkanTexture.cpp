#include "Vulkan.hpp"

//Creates a Vulkan Texture object, loading from the file specified.
VulkanTexture::VulkanTexture(	Vulkan* vulkan,
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

	vulkan->create_vulkan_image(	converted_surface->w, converted_surface->h,
									VK_FORMAT_B8G8R8A8_UINT,
									texture_image, device_memory);

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
}

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

