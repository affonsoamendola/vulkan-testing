#include <iostream>

#include "SDL2/SDL.h"

#include "Input.hpp"
#include "Rect.hpp"

//INPUT SYSTEM CLASS MEMBER FUNCTIONS:
//Creates Input Subsystem.
Input::Input()
{
	//SDL_SetRelativeMouseMode(SDL_TRUE);
	memset(key_down, 0, 284);
}

//Called once every frame, updates key states, and does polling on keyboard events.
void Input::update()
{
	ptr_keyboard_state 	=	SDL_GetKeyboardState(NULL); 
	mouse_state 		=	SDL_GetMouseState(&mouse_x, &mouse_y);

	while(SDL_PollEvent(&(event)))
	{
		if( event.type == SDL_MOUSEMOTION )
		{
			on_mouse_movement(event.motion.xrel, event.motion.yrel);
		}

		if( event.type == SDL_MOUSEBUTTONUP)
		{
			mouse_left_down = false;
			check_mouse_events(INPUT_CLICK_AREA, mouse_x, mouse_y);
		}

		if( event.type == SDL_MOUSEBUTTONDOWN)
		{
			mouse_left_down = true;
		}

		if(	event.type == SDL_KEYDOWN )
		{
			key_down[SDL_GetScancodeFromKey(event.key.keysym.sym)] = true;

			if(event.key.repeat == 0) check_command_list_keys(event.key.keysym.sym, registered_key_down_commands);
		}

		if(	event.type == SDL_KEYUP )
		{
			key_down[SDL_GetScancodeFromKey(event.key.keysym.sym)] = false;

			check_command_list_keys(event.key.keysym.sym, registered_key_up_commands);
		}

		if(event.type == SDL_QUIT)
		{
			signal_quit();
			break;
		}
	}

	if(mouse_left_down) check_mouse_events(INPUT_HOLD_AREA, mouse_x, mouse_y);
	check_mouse_events(INPUT_HOVER_AREA, mouse_x, mouse_y);
	check_held_keys();
}

//Registers a command to be executed everytime an event_type happens.
bool Input::register_command(InputEventType event_type, Command command)
{
	switch(event_type)
	{
		case INPUT_KEY_DOWN:
			registered_key_down_commands.push_back(command);
			return true;
		case INPUT_KEY_UP:
			registered_key_up_commands.push_back(command);
			return true;
		case INPUT_KEY_HOLD:
			registered_key_held_commands.push_back(command);
			return true;
		case INPUT_MOUSE_MOVEMENT:
			registered_mouse_movement_commands.push_back(command);
			return true;
		default:
			return false;
	}
}

bool Input::register_area(AreaEvent area_event)
{
	switch(area_event.event_type)
	{
		case INPUT_CLICK_AREA:
			registered_click_areas.push_back(area_event);
			return true;
		case INPUT_HOLD_AREA:
			registered_hold_areas.push_back(area_event);
			return true;
		case INPUT_HOVER_AREA:
			registered_hover_areas.push_back(area_event);
			return true;
	}
	return false;
}

//Checks if theres a command in the command list assigned to that 
//keycode on the key map
void Input::check_command_list_keys(SDL_Keycode keycode, const std::vector<Command>& registered_key_commands)
{
	CommandType command_type = key_map[keycode];

	if(command_type == NO_COMMAND) return;

	for(auto command : registered_key_commands)
	{
		if(command.command_type == command_type)
		{ 
			command.f_execute(command.ptr_user_data);
			break;
		}
	}
}

//Checks all the held keys for some command that is assigned to them, and executes them.
void Input::check_held_keys()
{
	for(int sc = 0; sc < 284; sc++)
	{
		if(key_down[sc])
		{
			check_command_list_keys(SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(sc)), registered_key_held_commands);
		}
	}
}

//Executes the commands bound to mouse movement, calls their functions with
//a MouseMovement struct pointer in the user_data, which includes the
//Delta Movement.
void Input::on_mouse_movement(int mouse_delta_x, int mouse_delta_y)
{
	MouseMovement mouse_movement(mouse_delta_x, mouse_delta_y);

	for(auto command : registered_mouse_movement_commands)
	{
		command.f_execute(&mouse_movement);
		break;
	}
}

//Called every mouse click, searchs all registered click areas for the highest
//layer rect on the mouse position when clicked and calls its function
void Input::check_mouse_events(InputEventType event_type, int mouse_x, int mouse_y)
{
	AreaEvent top_layer;
	bool found_area = false;

	std::vector<AreaEvent>* relevant_registry;

	switch(event_type)
	{
		case INPUT_CLICK_AREA:
			relevant_registry = &registered_click_areas;
			break;
		case INPUT_HOVER_AREA:
			relevant_registry = &registered_hover_areas;
			break;
		case INPUT_HOLD_AREA:
			relevant_registry = &registered_hold_areas;
			break;
	}


	for(auto area : *relevant_registry)
	{
		if(area.rect.is_inside(mouse_x, mouse_y))
		{
			if(found_area == false) 
			{
				top_layer = area;
				found_area = true;
			}
			else
			{
				if(area.layer >= top_layer.layer) top_layer = area;
			}
		}
	}

	if(found_area) top_layer.f_execute(top_layer.ptr_user_data);
}

//------------------------
