#include "Vulkan.hpp"

//Creates a Vulkan Texture object, loading from the file specified.
VulkanTexture::VulkanTexture(	Vulkan* vulkan,
								const char* texture_file)
{
	vulkan_instance = vulkan;
	image_format = VK_FORMAT_B8G8R8A8_UINT;

	SDL_Surface* img_surface = IMG_Load(texture_file);
	uint32_t width = img_surface->w;
	uint32_t height = img_surface->h;

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

	vulkan->create_vulkan_image(	width, height,
									image_format,
									image, device_memory);

	VkDeviceSize image_size = width * height * 4;

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;

	vulkan->create_buffer(	image_size, 
							VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
							staging_buffer, staging_buffer_memory);

	void* data;

	vkMapMemory(vulkan->logical_device, staging_buffer_memory, 0, image_size, 0, &data);
		memcpy(data, converted_surface->pixels, static_cast<size_t>(image_size));
	vkUnmapMemory(vulkan->logical_device, staging_buffer_memory);

	SDL_FreeSurface(converted_surface);

	vulkan->exec_copy_buffer_to_image_cmd(	staging_buffer, image,
											width, height);

	image_extent = {width, height};


	vkDestroyBuffer(vulkan->logical_device, staging_buffer, nullptr);
	vkFreeMemory(vulkan->logical_device, staging_buffer_memory, nullptr);
}

//Destroys a texture object
VulkanTexture::~VulkanTexture()
{
	vkDestroyImage(vulkan_instance->logical_device, image, nullptr);
	vkFreeMemory(vulkan_instance->logical_device, device_memory, nullptr);
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

