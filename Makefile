
BINARY = vulkan
OBJS = $(subst .cpp,.o, $(wildcard *.cpp))
VULKAN_SDK_PATH = /home/affonso/opt/vulkansdk/x86_64
CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include -g
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs sdl2` -lvulkan -lSDL2_image

%.o: %.cpp
	g++ $(CFLAGS) $< -c -o $@ $(LDFLAGS)

$(BINARY) : $(OBJS)
	g++ $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(BINARY) 

.PHONY: test clean

run: $(BINARY)
	./$(BINARY)

clean:
	rm -f $(BINARY)
	rm -f *.o