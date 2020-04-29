BINARY := vulkan
SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(patsubst src/%.cpp,src/%.o,$(SOURCES))
DEPENDS := $(patsubst src/%.cpp,src/%.d,$(SOURCES))
CXXFLAGS := -std=c++17 -I$(VULKAN_SDK_PATH)/include -I./include -g
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs sdl2` -lvulkan -lSDL2_image
SHADERSSOURCES := shaders/frag.frag shaders/vert.vert
SHADERSOBJ := shaders/frag.spv shaders/vert.spv
# ADD MORE WARNINGS!
WARNING := 

# .PHONY means these rules get executed even if
# files of those names exist.
.PHONY: all clean

# The first rule is the default, ie. "make",
# "make all" and "make parking" mean the same
all: $(BINARY) $(SHADERSOBJ)

%.spv: %.frag
	glslc $^ -o $@

%.spv: %.vert
	glslc $^ -o $@

run: $(BINARY) $(SHADERSOBJ)
	./$(BINARY)

clean:
	$(RM) *.o *.d $(BINARY) $(SHADERSOBJ)

# Linking the executable from the object files
$(BINARY): $(OBJECTS)
	$(CXX) $(WARNING) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

-include $(DEPENDS)

%.o: %.cpp Makefile
	$(CXX) $(WARNING) $(CXXFLAGS) $(LDFLAGS) -MMD -MP -c $< -o $@