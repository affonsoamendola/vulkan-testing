VULKAN_SDK_PATH = /home/affonso/opt/vulkansdk/x86_64
CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan

vulkantest: main.cpp VulkanHolder.cpp
	g++ $(CFLAGS) -o vulkantest main.cpp VulkanHolder.cpp $(LDFLAGS)

.PHONY: test clean

run: vulkantest
	./vulkantest

clean:
	rm -f vulkantest