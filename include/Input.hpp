#pragma once

//This file was written under the influence of 
//New Order's Bizarre Love Triangle (Extended Dance Mix)
//Give it a listen <3

#include <vector>
#include <map>
#include <iostream>

#include "SDL2/SDL.h"
#include "Rect.hpp"

//All Input Event types. Includes some not used by SDL
enum InputEventType
{
	INPUT_KEY_DOWN,
	INPUT_KEY_UP,
	INPUT_KEY_HOLD,
	INPUT_MOUSE_MOVEMENT,
	INPUT_CLICK_AREA,
	INPUT_HOVER_AREA,
	INPUT_ENTER_AREA,
	INPUT_LEAVE_AREA,
	INPUT_HOLD_AREA
};

//All CommandTypes
enum CommandType
{
	NO_COMMAND = 0,
	QUIT,
	TEST
};

//Command class definition.
//A command has a name, a function pointer and a void pointer for the 
//data the execute function could use.
struct Command
{
	Command(CommandType t_command, 
			void (*t_execute)(void * user_data), 
			void * t_user_data)
	{
		command_type = t_command;
		f_execute = t_execute;
		ptr_user_data = t_user_data;
	}

	CommandType command_type;
	void (*f_execute)(void * user_data) = nullptr;
	void * ptr_user_data = nullptr;
};

struct AreaEvent
{
	Rect rect;
	int layer;

	InputEventType event_type;

	void (*f_execute)(void * user_data) = nullptr;
	void * ptr_user_data = nullptr;
};

struct MouseMovement
{
	MouseMovement(int del_x, int del_y) : delta_x(del_x), delta_y(del_y) {}
	int delta_x;
	int delta_y;
};

class Input
{
	SDL_Event event;

	const unsigned char * ptr_keyboard_state;

	bool key_down[284];

	int mouse_state;

	int mouse_x;
	int mouse_y;

	bool mouse_left_down = false;
	bool mouse_right_down = false;

	//Assigns a Keycode for a command, im not really happy with this.]
	//TODO: Find better way to map Keycodes to CommandTypes
	std::map<SDL_Keycode, CommandType> key_map = 
	{
		{SDLK_ESCAPE, QUIT},
	};

	//Registered command vectors. 
	//You can register a command to be executed by calling register_command. 
	//Not sure if having a separate vector for each type of event is a good idea.
	//TODO: See if theres is a better way (or even need) to change this.
	std::vector<Command> registered_key_down_commands;
	std::vector<Command> registered_key_held_commands;
	std::vector<Command> registered_key_up_commands;
	std::vector<Command> registered_mouse_movement_commands;

	std::vector<AreaEvent> registered_click_areas;
	std::vector<AreaEvent> registered_hover_areas;
	std::vector<AreaEvent> registered_enter_areas;
	std::vector<AreaEvent> registered_leave_areas;
	std::vector<AreaEvent> registered_hold_areas;
	
public:
	Input();

	void update();	

	bool register_command(InputEventType event_type, Command command);
	bool register_area(AreaEvent area_event);

	void check_command_list_keys(SDL_Keycode keycode, const std::vector<Command>& registered_key_commands);
	
	void on_mouse_movement(int mouse_x, int mouse_y);

	void check_mouse_events(InputEventType event_type, int mouse_x, int mouse_y);
	void check_held_keys();
};

extern void signal_quit(void * user_data = nullptr);