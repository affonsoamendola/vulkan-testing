glslc ./shaders/frag.frag -o shaders/frag.spv
glslc ./shaders/vert.vert -o shaders/vert.spv
g++ -std=c++17 -Iinclude -I$VULKAN_SDK/vulkan/include -I/usr/include/SDL2 -L$VULKAN_SDK/lib -lm -lSDL2 -lSDL2_image -lvulkan src/*.cpp -o vulkan
