#pragma once
#include <windows.h>
#include <string>
#include <chrono>
#include <iostream>
#include <queue>
#include <stack>
#include "resource.h"
#include <Commdlg.h>
#include "ImageType.h"


#include <cstdlib>
#include <cwchar>
#include<winbase.h>



#include <strsafe.h>
#define WM_NAME (WM_USER + 1)
#define ENEM_NB_ROW 3
#define ENEM_NB_COL 9
#define ENEM_MARG 10

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


	HBRUSH m_background; /// -------
	COLORREF color = RGB(255, 255, 255);


	HBRUSH enemy_brush;
	HBRUSH player_brush;
	HBRUSH bullet_brush;

	int left;
	int top;

	int enemy_status = 0;
	int enemy_moves = 0;
	
	POINT size = { 800,600 };  //     -------
	POINT enemy_size = { 50, 40 };
	POINT player_size = { 50, 50 };
	POINT bullet_size = { 5, 15 };

	POINT actual_size; //  policzyc
	int offset = 30;
	int moves_goal = ENEM_NB_COL * ((enemy_size.x / offset) - 5);
	
	POINT start_enemies;
//	POINT enemy_pos;// 

	POINT enemies_pos[ENEM_NB_ROW][ENEM_NB_COL];
	POINT player_pos;//

	POINT enemies_pos_bottom[ENEM_NB_COL];
	HWND enemies_bottom[ENEM_NB_COL];

	HWND player;
//	HWND enemy;
	HWND enemies[ENEM_NB_ROW][ENEM_NB_COL];


	std::queue<HWND> bullets;
	std::queue<POINT> positions;
	int bullet_counter = 0;

	HBITMAP player_sprite_bitmap;
	HBITMAP enemy_sprite_bitmap;

	int player_animation = 0;
	int enemy_animation = 0;


	
	HMENU menu;
	CHOOSECOLOR choose_color;
	COLORREF custom_colors[16];
	

	HBITMAP main_background = NULL; //     ------- 
	wchar_t file_name[MAX_PATH]; //    ------
	OPENFILENAME open_file; //    odtworzyc
	BITMAP bitmap_info; //    policzyc

	POINT center_image_pos = {-1,-1};

	
	ImageType image_type = Fit;//   -----
	int score; //     -----


	LPCWSTR iniFilePath = L"C:\\Users\\przem\\Pulpit\\save.ini"; ///////// uwaga tutaj

	HWND end_window;
	int endgame = 0;

	wchar_t name[100];

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
	bool check_bullet(POINT bullet);
	void destroy_window(int i, int j);
	void create_bullet();

	void draw_sprite_player();
	void draw_sprite_enemy();

	void on_command(WPARAM wparam);
	void set_new_pos();
	void update_overlay(HDC main_context);
	void draw_and_calc_overlay();
	void calc_image_pos();
	void draw_score();
	void save();
	void load();

	void on_game_end();
	void create_and_show_end_window();
	


	static INT_PTR CALLBACK end_window_proc(
		HWND window, UINT message,
		WPARAM wparam, LPARAM lparam);

public:
	game(HINSTANCE instance);
	int run(int show_command);
};