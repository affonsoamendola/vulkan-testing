#include "Vulkan.hpp"

void Vulkan::create_descriptor_set_layout()
{
	VkDescriptorSetLayoutBinding ubo_layout_binding = {};

	ubo_layout_binding.binding = 0;
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	ubo_layout_binding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layout_info = {};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = 1;
	layout_info.pBindings = &ubo_layout_binding;

	if (vkCreateDescriptorSetLayout(logical_device, &layout_info, nullptr, &descriptor_set_layout) != VK_SUCCESS) 
	{
    	throw std::runtime_error("Failed to create descriptor set layout.");
	}
}

void Vulkan::create_descriptor_pool()
{
	VkDescriptorPoolSize pool_size = {};
	pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_size.descriptorCount = static_cast<uint32_t>(swap_chain_images.size());

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = 1;
	pool_info.pPoolSizes = &pool_size;
	pool_info.maxSets = static_cast<uint32_t>(swap_chain_images.size());

	if (vkCreateDescriptorPool(logical_device, &pool_info, nullptr, &descriptor_pool) != VK_SUCCESS) 
	{
    	throw std::runtime_error("Failed to create descriptor pool.");
	}
}

void Vulkan::create_descriptor_sets()
{
	std::vector<VkDescriptorSetLayout> layouts(swap_chain_images.size(), descriptor_set_layout);
	VkDescriptorSetAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = descriptor_pool;
	alloc_info.descriptorSetCount = static_cast<uint32_t>(swap_chain_images.size());
	alloc_info.pSetLayouts = layouts.data();

	descriptor_sets.resize(swap_chain_images.size());

	if (vkAllocateDescriptorSets(logical_device, &alloc_info, descriptor_sets.data()) != VK_SUCCESS) 
	{
    	throw std::runtime_error("Failed to allocate descriptor sets.");
	}

	for (size_t i = 0; i < swap_chain_images.size(); i++) 
	{
	 	VkDescriptorBufferInfo buffer_info = {};
	    buffer_info.buffer = uniform_buffers[i];
	    buffer_info.offset = 0;
	    buffer_info.range = sizeof(UniformBufferObject);

	    VkWriteDescriptorSet descriptor_write = {};
		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.dstSet = descriptor_sets[i];
		descriptor_write.dstBinding = 0;
		descriptor_write.dstArrayElement = 0;

		descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_write.descriptorCount = 1;

		descriptor_write.pBufferInfo = &buffer_info;
		descriptor_write.pImageInfo = nullptr; // Optional
		descriptor_write.pTexelBufferView = nullptr; // Optional

		vkUpdateDescriptorSets(logical_device, 1, &descriptor_write, 0, nullptr);
	}
}