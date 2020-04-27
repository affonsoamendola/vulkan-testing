#pragma once

//Holds the device extensions we'll need.
const std::vector<const char*> required_device_extensions =
{

    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

//Holds the queue family indices.
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return  graphicsFamily.has_value() && 
                presentFamily.has_value();
    }
};
