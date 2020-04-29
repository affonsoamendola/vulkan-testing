#pragma once

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

struct VulkanSpriteQueue
{
	int queued_layers = 0;
	std::vector<std::vector<VulkanSprite*>> queue;

	void queue_sprite(VulkanSprite* ptr_sprite, uint32_t layer);
	void clear_queue();
};
