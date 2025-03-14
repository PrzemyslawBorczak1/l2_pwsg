#pragma once
#include <windows.h>
#include <string>
#include <chrono>
#include<iostream>
#include <array>

class game
{
private:
	POINT m_screen_size;
	bool register_class();

	static std::wstring const s_class_name;
	static constexpr UINT_PTR s_timer = 1;
	static constexpr UINT_PTR bullet_timer = 2;

	HINSTANCE m_instance; // instancja execa
	HWND m_main; /// uchwyt okna
	HBRUSH m_background;
	HBRUSH enemy_brush;
	HBRUSH player_brush;
	int left;
	int top;

	int enemy_status = 0;

	POINT size = { 800,600 };
	POINT enemy_size = { 50, 40 };
	POINT player_size = { 50, 50 };

	POINT enemy_pos;
	POINT player_pos;


	HWND player;
	HWND enemy;

	static LRESULT CALLBACK window_proc_static(
		HWND window, UINT message,
		WPARAM wparam, LPARAM lparam);

	LRESULT window_proc(
		HWND window, UINT message,
		WPARAM wparam, LPARAM lparam);

	HWND create_window();
	void on_activ(WPARAM wparam);
	INT_PTR on_colorstatic(LPARAM lparam);

	void on_keydown(WPARAM wparam);
	void on_timer(WPARAM wparam);
	void move_enemy();
	void move_bullets();
	void create_bullet();
	HWND bullets[4];
	POINT positions[4];

public:
	game(HINSTANCE instance);
	int run(int show_command);
};