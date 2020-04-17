#include "VulkanHolder.hpp"

#include <iostream>
#include <stdexcept>

int main()
{
    try
    {
        VulkanHolder vulkanHolder;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}