#pragma once
#include <windows.h>
#include <string>
#include <chrono>
#include <iostream>
#include <list>
#include "resource.h"
#include <Commdlg.h>
#include "ImageType.h"

class game
{
private:
	POINT m_screen_size;
	bool register_class();

	static std::wstring const s_class_name;
	static constexpr UINT_PTR s_timer = 1;
	static constexpr UINT_PTR bullet_timer = 2;
	static constexpr UINT_PTR player_sprite_timer = 3;
	static constexpr UINT_PTR enemy_sprite_timer = 4;

	HINSTANCE m_instance; // instancja execa
	HWND m_main; /// uchwyt okna
	HBRUSH m_background;
	HBRUSH enemy_brush;
	HBRUSH player_brush;
	HBRUSH bullet_brush;

	int left;
	int top;

	int enemy_status = 0;

	POINT size = { 800,600 };
	POINT enemy_size = { 50, 40 };
	POINT player_size = { 50, 50 };
	POINT bullet_size = { 5, 15 };

	POINT actual_size;
	int offset = 30;

	POINT enemy_pos;
	POINT player_pos;


	HWND player;
	HWND enemy;

	std::list<HWND> bullets;
	std::list<POINT> positions;

	HBITMAP player_sprite_bitmap;
	HBITMAP enemy_sprite_bitmap;
	HBITMAP main_background;

	int player_animation = 0;
	int enemy_animation = 0;


	//
	HMENU menu;
	CHOOSECOLOR choose_color;
	COLORREF custom_colors[16];

	wchar_t file_name[MAX_PATH];
	OPENFILENAME open_file;
	BITMAP bitmap_info;

	POINT image_pos;

	
	ImageType image_type = Center;

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

	void draw_sprite_player();
	void draw_sprite_enemy();

	void on_command(WPARAM wparam);
	void calc_new_pos();
	void background_color();
	void background_image();
	void draw_overlay(HDC main_context);
	void calc_image_pos();

public:
	game(HINSTANCE instance);
	int run(int show_command);
};