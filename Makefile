SOURCES = main.cpp Vulkan.cpp VulkanTexture.cpp VulkanImage.cpp Input.cpp
VULKAN_SDK_PATH = /home/affonso/opt/vulkansdk/x86_64
CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include -g
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs sdl2` -lvulkan

vulkantest: $(SOURCES)
	g++ $(CFLAGS) -o vulkantest $(SOURCES) $(LDFLAGS)

.PHONY: test clean

run: vulkantest
	./vulkantest

clean:
	rm -f vulkantest