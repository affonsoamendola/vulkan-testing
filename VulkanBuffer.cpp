#include "Vulkan.hpp"

void Vulkan::create_buffer(   	VkDeviceSize size, VkBufferUsageFlags usage,
								VkMemoryPropertyFlags properties,
								VkBuffer& buffer, VkDeviceMemory& memory)
{
	VkBufferCreateInfo buffer_info = {};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = size;
	buffer_info.usage = usage;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if(vkCreateBuffer(logical_device, &buffer_info, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create buffer!");
	}

	VkMemoryRequirements mem_requirements;
	vkGetBufferMemoryRequirements(logical_device, buffer, &mem_requirements);

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_requirements.size;
	alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties);

	if(vkAllocateMemory(logical_device, &alloc_info, nullptr, &memory) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Buffer memory!");
	}

	vkBindBufferMemory(logical_device, buffer, memory, 0);
}