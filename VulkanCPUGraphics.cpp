#include "Vulkan.hpp"

#include "Matrix4f.hpp"

void Vulkan::draw_text(	VulkanFont& font,
						const char * content,
						const VkOffset2D& offset,
						uint32_t layer)
{
	VkOffset2D current_offset;
	current_offset.y = offset.y;
	for(size_t i = 0; content[i] != '\0'; i++)
	{
		current_offset.x = offset.x + i*font.dimensions.width;

		draw_char(font, content[i], current_offset, layer);
	}
}

void Vulkan::draw_char(	VulkanFont& font,
						char content,
						const VkOffset2D& offset,
						uint32_t layer)
{
	char printable = content - 0x20;

	VkRect2D src =	{	(printable%0x20)*(int32_t)font.dimensions.width, 
						(printable/0x20)*(int32_t)font.dimensions.height, 
						font.dimensions.width, 
						font.dimensions.height};
	VkRect2D dst = {	offset.x, offset.y, 
						font.dimensions.width, font.dimensions.height};

	VulkanSprite* sprite = new VulkanSprite(&font.texture, 
											src, 
											dst);
	sprite_queue.queue_sprite(sprite, layer);
}

void Vulkan::update_uniform_buffer(uint32_t current_image)
{
	UniformBufferObject ubo = {};
	ubo.model = m4f_rotate(m4f_identity(), Timer::time() * 0.5f, Vector3f(0.8f, 0.0f, 0.5f));
	ubo.view  = m4f_translate(Vector3f(0.0f, 0.0f, 3.0f));
	ubo.proj  = m4f_perspective(radians(45.0f), (float) WIDTH / (float) HEIGHT, 0.1f, 10.0f);

	void* data;
	vkMapMemory(logical_device, uniform_buffers_memory[current_image], 0, sizeof(ubo), 0, &data);
    	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(logical_device, uniform_buffers_memory[current_image]);
}
