#pragma once

struct Rect
{
	Rect(int t_x = 0, int t_y = 0, int t_size_x = 0, int t_size_y = 0) 
		: x(t_x), y(t_y), size_x(t_size_x), size_y(t_size_y) {}

	int x = 0;
	int y = 0;
	int size_x = 0;
	int size_y = 0;

	bool is_inside(int t_x, int t_y)
	{
		return	t_x >= x && t_x < x + size_x &&
				t_y >= y && t_y < y + size_y;
	}
};