#include "Vulkan.hpp"
#include "Input.hpp"

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

#include <iostream>
#include <stdexcept>

bool is_game_running = true;

Input* input;
Vulkan* vulkan;

void init()
{
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        throw std::runtime_error("Could not initialize SDL2.");
    }

    if(IMG_Init(SDL_INIT_EVERYTHING) < 0)
    {
        throw std::runtime_error("Could not initialize SDL2_image.");
    }

    vulkan = new Vulkan();
    input = new Input();
}

void main_loop()
{
    while(is_game_running)
    {
        input->update();
        vulkan->update();
    }
}   

void cleanup()
{
    delete input;
    delete vulkan;
}

int main()
{
    try
    {
        init();
        main_loop();
        cleanup();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void signal_quit(void * user_data)
{

    is_game_running = false;
}

