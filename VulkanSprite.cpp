#include "Vulkan.hpp"

VulkanSprite::VulkanSprite(	VulkanTexture* t_ptr_texture, 
							VkRect2D t_source, 
							VkRect2D t_destination, 
							bool t_pixel_perfect)
{
	ptr_texture = t_ptr_texture;
	source = t_source;
	destination = t_destination;
	pixel_perfect = t_pixel_perfect;
}

void VulkanSpriteQueue::queue_sprite(	VulkanSprite* ptr_sprite, 
										uint32_t layer)
{
	if(layer >= queued_layers)
	{
		queue.resize(layer + 1);
		queued_layers = layer;
	}

	(queue[layer]).push_back(ptr_sprite);
}

void VulkanSpriteQueue::clear_queue()
{
	for(auto layer : queue)
	{
		for(auto ptr_sprite : layer)
		{
			delete ptr_sprite;
		}
		
		layer.clear();
	}

	queue.clear();
}