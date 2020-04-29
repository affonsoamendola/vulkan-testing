#include "Vulkan.hpp"

//Initializes a VkImage and allocates memory for it in the device.
void Vulkan::create_vulkan_image(   uint32_t width, uint32_t height, 
                                    VkFormat format, 
                                    VkImageUsageFlags usage,
                                    VkImage& image, VkDeviceMemory& memory)
{
    VkImageCreateInfo create_info = {};

    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.format = format;
    create_info.extent = {  width, 
                            height, 1};
    create_info.mipLevels = 1;
    create_info.arrayLayers = 1;
    create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (vkCreateImage(logical_device, &create_info, nullptr, &image) != VK_SUCCESS)
    {
         throw std::runtime_error("Failed to create image handle.");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(logical_device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(physical_device, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(logical_device, &allocInfo, nullptr, &memory) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(logical_device, image, memory, 0);
}

VkImageView Vulkan::create_image_view(    VkImage image, 
                                          VkFormat format) 
{
    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VkImageView image_view;

    if (vkCreateImageView(logical_device, &view_info, nullptr, &image_view) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create texture image view!");
    }

    return image_view;
}
