#include "Vulkan.hpp"
#include "VulkanTexture.hpp"

void Vulkan::create_texture_sampler()
{
	VkSamplerCreateInfo sampler_info = {};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = VK_FILTER_LINEAR;
	sampler_info.minFilter = VK_FILTER_LINEAR;
	sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.anisotropyEnable = VK_TRUE;
	sampler_info.maxAnisotropy = 16;
	sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	sampler_info.unnormalizedCoordinates = VK_FALSE;
	sampler_info.compareEnable = VK_FALSE;
	sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
	sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_info.mipLodBias = 0.0f;
	sampler_info.minLod = 0.0f;
	sampler_info.maxLod = 0.0f;

	 if (vkCreateSampler(logical_device, &sampler_info, nullptr, &texture_sampler) != VK_SUCCESS) 
	 {
        throw std::runtime_error("Failed to create texture sampler!");
    }
}


//Creates a Vulkan Texture object, loading from the file specified.
VulkanTexture::VulkanTexture(	Vulkan* vulkan,
								const char* texture_file)
{
	vulkan_instance = vulkan;
	image_format = VK_FORMAT_B8G8R8A8_SRGB;
	
	//Loads image file
	SDL_Surface* img_surface = IMG_Load(texture_file);
	uint32_t width = img_surface->w;
	uint32_t height = img_surface->h;

	if(img_surface == NULL)
	{
		std::cerr << "Error opening texture file: " << texture_file << " " << SDL_GetError() << std::endl;
		throw std::runtime_error("Error opening texture file.");
	}

	SDL_PixelFormat* sdl_format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
	
	//Converts file to specified format.
	SDL_Surface* converted_surface = SDL_ConvertSurface(img_surface, sdl_format, 0);

	if(converted_surface == NULL)
	{
		std::cerr << "Error converting texture file to RGBA8888: " << texture_file << " " << SDL_GetError() << std::endl;
		throw std::runtime_error("Error converting texture file.");
	}

	//Frees now unneeded memory.
	SDL_FreeFormat(sdl_format);
	SDL_FreeSurface(img_surface);

	//Creates the vulkan image we need.
	vulkan->create_vulkan_image(	width, height,
									image_format,
									VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
									VK_IMAGE_USAGE_TRANSFER_DST_BIT |
									VK_IMAGE_USAGE_SAMPLED_BIT,
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

	vulkan->exec_transition_image_layout_cmd(  	image, image_format, 
		                                        VK_IMAGE_LAYOUT_UNDEFINED,
		                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	//Copies buffer to image.
	vulkan->exec_copy_buffer_to_image_cmd(	staging_buffer, image,
											width, height);

	vulkan->exec_transition_image_layout_cmd(  	image, image_format, 
		                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		                                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	image_extent = {width, height};

	vkDestroyBuffer(vulkan->logical_device, staging_buffer, nullptr);
	vkFreeMemory(vulkan->logical_device, staging_buffer_memory, nullptr);

	image_view = vulkan->create_image_view(image, VK_FORMAT_B8G8R8A8_SRGB);

}

//Destroys a texture object
VulkanTexture::~VulkanTexture()
{
	vkDestroyImageView(vulkan_instance->logical_device, image_view, nullptr);
	vkDestroyImage(vulkan_instance->logical_device, image, nullptr);
	vkFreeMemory(vulkan_instance->logical_device, device_memory, nullptr);
}
