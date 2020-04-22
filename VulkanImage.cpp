#include "vulkan/vulkan.h"

//Initializes a VkImage and allocates memory for it in the device.
void createVulkanImage( VkPhysicalDevice physicalDevice, VkDevice device, 
                        uint32_t width, uint32_t height, 
                        VkFormat format,
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
    create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (vkCreateImage(device, &create_info, nullptr, &image) != VK_SUCCESS)
    {
         throw std::runtime_error("Failed to create image handle.");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, image, memory, 0);
}