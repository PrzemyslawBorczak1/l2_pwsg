#include "game.h"


std::wstring const game::s_class_name{ L"Space Invaders" };

bool game::register_class() {// rejestrowanie klasy czyli zapisanie co bedzie obslugiwac okienko 
	WNDCLASSEXW desc{}; // stworzenie struktury opisujacej klase do rejestracji

	if (GetClassInfoExW(m_instance, s_class_name.c_str(),
		&desc) != 0) return true; /// sprawdzenie czy klasa istnieje  


	desc = { .cbSize = sizeof(WNDCLASSEXW), // smieszne zabezpieczenie		!!!
	.lpfnWndProc = window_proc_static, // statyczna funkcja obslugi wiadomosci funkcja	!!!
	.hInstance = m_instance,     // instantcja klasy					!!!

	.hCursor = LoadCursorW(nullptr, IDC_ARROW), // ladowanie kursora strzalka
	.hbrBackground = m_background, // tlo okienka   cos moze sie psuc jak sie tego nie ustawi

	.lpszMenuName = MAKEINTRESOURCEW(ID_MAINMENU), 
			//jesli menu jes dodane to tak sie je podlacza po id tu nie jest dodane

		.lpszClassName = s_class_name.c_str() }; // nazwa klasy wazne		!!!

	return RegisterClassExW(&desc) != 0; // zarejestrowanie klasy (0 jak sie nie powiodlo)
}

HWND game::create_window()
{

	HWND window = CreateWindowExW(
		WS_EX_TOPMOST | WS_EX_LAYERED ,   // dodatkowe style   scrol bar,    overllaping .... 
		s_class_name.c_str(), // klasa wczesniej zerejestrowana    c_str() zmienia na zwykly string z C
		L"Space Invaders _ WINAPI", // nazwa okienka

		WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
		WS_BORDER | WS_MINIMIZEBOX, /// style podstawowe   
		// overlapped - typowe okno?, sysmenu - typowe opcje zwiekszenie zmiejszenie,
		// caption czy ma typowy pasek nazwy, 


		left, top, // pozycja usedefault robi na (0,0) 
		size.x, size.y, // rozmiar (domyslny)
		nullptr, // rodzic 
		nullptr, /// Menu handle
		m_instance, // instantcja zapisana przy rejestrrowaniu klasy
		this); // customowy handle

	SetLayeredWindowAttributes(m_main, 0, 255, LWA_ALPHA);

	start_enemies ={ (size.x - enemy_size.x) / 2  - (ENEM_NB_COL / 2) * (enemy_size.x + ENEM_MARG ),  enemy_size.y - 20 };// nie do konca dobrze liczone

	for (int i = 0; i < ENEM_NB_ROW; i++) {
		for (int j = 0; j < ENEM_NB_COL; j++) {
			enemies_pos[i][j] = { start_enemies.x + j * (enemy_size.x + ENEM_MARG),   start_enemies.y + i * (enemy_size.y + ENEM_MARG) };
			enemies[i][j] = CreateWindowExW(
				0,
				L"STATIC",
				nullptr, ///  brak nazwy klasy sprawia ze okno powstaje z puli gotowych   czyli basic wyglad
				WS_CHILD | WS_VISIBLE | SS_CENTER,
				/// CHILD czyli nie wychodzi poza ramke?,       WS_VISIBLE okno poajawi sie bez show window     
				// CENTER  wyrownanie tekstu w oknie poziomo  do centrum
				enemies_pos[i][j].x, enemies_pos[i][j].y,// pozycja
				enemy_size.x, // rozmiar poziomo
				enemy_size.y, // rozmair pionowo
				window,
				nullptr,
				m_instance, /// klasa obslugujaca polecenia to tez glowna
				nullptr);

		}
	}

	

	player_pos = { (size.x - player_size.x) / 2, size.y - 2 * player_size.y - 20 };
	player = CreateWindowExW(
		0,
		L"STATIC",
		nullptr, ///  brak nazwy klasy sprawia ze okno powstaje z puli gotowych   czyli basic wyglad
		WS_CHILD | WS_VISIBLE | SS_CENTER,
		/// CHILD czyli nie wychodzi poza ramke?,       WS_VISIBLE okno poajawi sie bez show window     
		// CENTER  wyrownanie tekstu w oknie poziomo  do centrum
		player_pos.x, player_pos.y,// pozycja
		player_size.x, // rozmiar poziomo
		player_size.y, // rozmair pionowo
		window,
		nullptr,
		m_instance, /// klasa obslugujaca polecenia to tez glowna
		nullptr);

	return window;
}

LRESULT game::window_proc_static( // statyczna funkcja obslugi wiadomosi. Jak przychodzi wiadomosc to najpierw do tej funkcji i ona rozsyala (ejst static dla calej klasy)
	HWND window, // okno ktore wyslalo (wiele okien moze miec ta sama funkcje)
	UINT message,	// jaka wiadomosc zostala wyslana np WM_TIMER
	WPARAM wparam, // pierwszy  argument wiadomosci 
	LPARAM lparam)  // drugi arg wiadomosci
{
	game* app = nullptr;
	if (message == WM_NCCREATE) // non user creation       wiadomosc wysylana po stworzeniu okna lparam to wskaznik do struktury
	{
		auto p = reinterpret_cast<LPCREATESTRUCTW>(lparam); // konwersja do struktury
		app = static_cast<game*>(p->lpCreateParams); // instancja klasy   bedzie w tej strukturze jesli byla podona ptrzy tworzeniu okna
		SetWindowLongPtrW(window, GWLP_USERDATA, /// przypisanie instancji klasy do handla okna
			reinterpret_cast<LONG_PTR>(app));
	}
	else
	{
		app = reinterpret_cast<game*>(
			GetWindowLongPtrW(window, GWLP_USERDATA)); // pobranie instancji klasy
	}

	if (app != nullptr)
	{
		return app->window_proc(window, message, /// podanie wiadomosci dalej do nie statycznej metody
			wparam, lparam);
	}
	return DefWindowProcW(window, message, wparam, lparam); // defoult zachowanie jesli brak instancji
}

LRESULT game::window_proc( /// procedura nie statyczna
	HWND window, UINT message,
	WPARAM wparam, LPARAM lparam)
{
	switch (message) { /// switchowanie sienpo waidomosci i wywolanie odpowiednich funkcji
	case WM_CLOSE: /// wcisniecie close przez uzytkowanika
		DestroyWindow(window); // defoult zachowanie znisczenie okna   zniszcenie wysyla WM_DESTROY
		return 0;
	case WM_DESTROY:
		PostQuitMessage(EXIT_SUCCESS); // zamkniecie messege loopa
		return 0;
	case WM_ACTIVATE:
		on_activ(wparam);
		return 0;
	case WM_CTLCOLORSTATIC:
		return on_colorstatic(lparam);
	case WM_KEYDOWN:
		on_keydown(wparam);
		return 0;
	case WM_TIMER: // jesli taimer jest ustawiony znaczy ze minal jego czas (timer dziala w loopie wysyla to oc 1 sekunde np)
		on_timer(wparam); /// btw ta wiadomosc w  wparam ma ktory timer wyslal tu nie potrzebne bo jest tylko 1 timer
		
		return 0;

	case WM_COMMAND:
		on_command(wparam);
		return 0;
	case WM_ERASEBKGND:
		update_overlay((HDC)wparam);

		return 1;
	case WM_PAINT:

		return DefWindowProcW(window, message, wparam, lparam);
	case WM_NAME:
		HWND htext = GetDlgItem(end_window, ID_TEXT);
		GetWindowText(htext, name, 100);
		EndDialog(end_window, 0);
		return 0;
		

		//case WM_CTLCOLORSTATIC: //kiedy cos ma byc narysowane
		//case WM_WINDOWPOSCHANGED: //zmiana pozycji okna 
		//case WM_TIMER:  // jesli taimer jest ustawiony znaczy ze minal jego czas (timer dziala w loopie wysyla to oc 1 sekunde np)
		//case WM_COMMAND: // menu klikniete
		//case WM_PAINT: // kiedy okno jest zmiejszane     albo sie odslania  czy cos
	}
	return DefWindowProcW(window, message, wparam, lparam); // switch nic nie zlapal
}

game::game(HINSTANCE instance) // instancja jest podawna przy wywolaniu tego smiesznego maina od WinApi
	: m_instance{ instance }/*klasy*/, m_main{}, m_screen_size{ GetSystemMetrics(SM_CXSCREEN),   /// obliczenie rozmiaru wyswietalacza
		GetSystemMetrics(SM_CYSCREEN) }, m_background{ CreateSolidBrush(RGB(255, 255, 255)) },
	enemy_brush{ CreateSolidBrush(RGB(255, 255, 255)) }, player_brush{ CreateSolidBrush(RGB(255, 0, 0)) },
	bullet_brush{ CreateSolidBrush(RGB(0, 0, 0)) }

{
	left = (m_screen_size.x - size.x) / 2;
	top = (m_screen_size.y - size.y) / 2;

	player_sprite_bitmap = LoadBitmap(m_instance, MAKEINTRESOURCE(ID_SHIP));

	enemy_sprite_bitmap = LoadBitmap(m_instance, MAKEINTRESOURCE(ID_INVADER));



	for (int i = 0; i < 16; ++i) {
		custom_colors[i] = RGB(255, 255, 255);
	}

	choose_color = {

		.lStructSize = sizeof(choose_color),
		.hwndOwner = m_main,  
		.rgbResult = RGB(255, 255, 255),
		.lpCustColors = custom_colors,
		.Flags = CC_RGBINIT | CC_FULLOPEN,
	};

	open_file = {
		.lStructSize = sizeof(OPENFILENAME),
		.hwndOwner = m_main,
		.lpstrFile = file_name,
		.nMaxFile = sizeof(file_name),
		.lpstrTitle = L"Choose Backgorund Image",
	};



	register_class();
	m_main = create_window();
	load();


	RECT rect;
	GetClientRect(m_main, &rect);
	actual_size.x = rect.right - rect.left;
	actual_size.y = rect.bottom - rect.top;
}

int game::run(int show_command) // show zommand tez arg z maina jak ma sie stworzyc okno
{

	menu = GetMenu(m_main);
	ShowWindow(m_main, show_command); /// pokazanie okna powikeszenie i gueess
	SetTimer(m_main, s_timer, 1000, nullptr); /// 1000 ms

	SetTimer(m_main, bullet_timer, 50, nullptr); /// 1000 ms
	SetTimer(m_main, player_sprite_timer, 40, nullptr); /// 1000 ms

	SetTimer(m_main, enemy_sprite_timer, 50, nullptr); /// 1000 ms

	MSG msg{};
	BOOL result = TRUE;
	while ((result = GetMessageW(&msg, nullptr, 0, 0)) != 0) /// loop message    peek gory 
		//arg: strukturka gdzie ma wpisac, okno od ktorego ma brac,  min i max eartosci wiadomosci
	{
		if (result == -1) // blad
			return EXIT_FAILURE;
		HACCEL shortcuts = LoadAcceleratorsW(m_instance,
			MAKEINTRESOURCEW(IDR_SHORTCUTS));

		if (!TranslateAcceleratorW(
			msg.hwnd, shortcuts, &msg))
		{

			TranslateMessage(&msg); // dla inutu z klawiatury 
			DispatchMessageW(&msg); /// odbieranie wiadomosci
		}
	}
	return EXIT_SUCCESS;
}

void game::on_activ(WPARAM wparam) {

	if (wparam == WA_INACTIVE) {
		SetLayeredWindowAttributes(m_main, 0, 40 * 255 / 100, LWA_ALPHA);
	}
	else
		SetLayeredWindowAttributes(m_main, 0, 255, LWA_ALPHA);

}

INT_PTR game::on_colorstatic(LPARAM lparam) {///////////
	/*if ((HWND)lparam == enemy)
		return reinterpret_cast<INT_PTR>(enemy_brush);
	else if((HWND)lparam == player)
		return reinterpret_cast<INT_PTR>(player_brush);
	else
		return reinterpret_cast<INT_PTR>(bullet_brush);*/


	//if ((HWND)lparam != player && (HWND)lparam != enemy)
	if((HWND)lparam != m_main)
		return reinterpret_cast<INT_PTR>(bullet_brush);

	//if ((HWND)lparam == enemy)
		//return reinterpret_cast<INT_PTR>(enemy_brush);

}

void game::on_keydown(WPARAM wparam) {
	if (VK_LEFT == wparam) {
		if (player_pos.x < 35)
			return;
		player_pos.x -= 25;
		MoveWindow(player, player_pos.x, player_pos.y, player_size.x, player_size.y, true);
	}
	else if (VK_RIGHT == wparam) {
		if (player_pos.x > size.x - 125) 
			return;
		player_pos.x += 25;
		MoveWindow(player, player_pos.x, player_pos.y, player_size.x, player_size.y, true);
	}
	else if (VK_SPACE == wparam) {
		create_bullet();
	}
}

void game::on_timer(WPARAM wparam) {
	switch (wparam) {
	case s_timer:
		move_enemy();
		return;
	case bullet_timer:
		move_bullets();
		return;
	case player_sprite_timer:

		player_animation++;
		player_animation %= 3;
		draw_sprite_player();
		return;
	case enemy_sprite_timer:

		enemy_animation++;
		enemy_animation %= 4;
		draw_sprite_enemy();
		return;

	}

}

void game::move_enemy() {
	if (0 == enemy_status) {
		for (int i = 0; i < ENEM_NB_ROW; i++) {
			for (int j = 0; j < ENEM_NB_COL; j++) {
				if (enemies[i][j] == NULL)
					continue;
				enemies_pos[i][j].x -= 25;
				MoveWindow(enemies[i][j], enemies_pos[i][j].x, enemies_pos[i][j].y, enemy_size.x, enemy_size.y, true);

			}
		}
		enemy_status = 1;
	}
	else if (1 == enemy_status) {
		for (int i = 0; i < ENEM_NB_ROW; i++) {
			for (int j = 0; j < ENEM_NB_COL; j++) {
				if (enemies[i][j] == NULL)
					continue;
				enemies_pos[i][j].x += 25;
				MoveWindow(enemies[i][j], enemies_pos[i][j].x, enemies_pos[i][j].y, enemy_size.x, enemy_size.y, true);

			}
		}

		enemy_status = 0;
	}
}

void game::move_bullets() {

	for(int l = 0; l < bullet_counter; l++){
		POINT it_pos = positions.front();
		positions.pop();

		it_pos.y -= offset;

		HWND it_bullets = bullets.front();
		bullets.pop();



		if (it_pos.y <= 0) {
			DestroyWindow(it_bullets);
			bullet_counter--;
			continue;
		}

		if (!check_bullet(it_pos)) {
			DestroyWindow(it_bullets);
			score++;
			bullet_counter--;
			draw_score();
			continue;
		}


		positions.push(it_pos);
		bullets.push(it_bullets);

		MoveWindow(it_bullets, it_pos.x, it_pos.y, bullet_size.x, bullet_size.y, true);
		
	}
}

bool game::check_bullet(POINT bullet) {
	for (int i = ENEM_NB_ROW - 1; i >= 0; i--) {
		for (int j = 0; j < ENEM_NB_COL; j++) {
			if (enemies[i][j] == NULL)
				continue;

			if (bullet.y <= enemies_pos[i][j].y + enemy_size.y &&
				bullet.x >= enemies_pos[i][j].x &&
				bullet.x <= enemies_pos[i][j].x + enemy_size.x)
			{
				destroy_window(i, j);
				return false;
			}
		}
	}
	return true;
}

void game::destroy_window(int i, int j) {
	/////

	//MoveWindow(enemies[i][j], 1000, 1000, 1, 1, true);
	DestroyWindow(enemies[i][j]);
	enemies_pos[i][j] = {0,0};
	enemies[i][j] = NULL;
}
void game::create_bullet() {
	positions.push({ player_pos.x + player_size.x / 2, player_pos.y - bullet_size.y });
	bullet_counter++;
	bullets.push(CreateWindowExW(
		0,
		L"STATIC",
		nullptr, ///  brak nazwy klasy sprawia ze okno powstaje z puli gotowych   czyli basic wyglad
		WS_CHILD | WS_VISIBLE | SS_CENTER,
		/// CHILD czyli nie wychodzi poza ramke?,       WS_VISIBLE okno poajawi sie bez show window     
		// CENTER  wyrownanie tekstu w oknie poziomo  do centrum
		player_pos.x + player_size.x / 2, player_pos.y - bullet_size.y,// pozycja

		bullet_size.x, bullet_size.y, // rozmiar (domyslny)
		m_main, // rodzic 
		nullptr, /// Menu handle
		m_instance, // instantcja zapisana przy rejestrrowaniu klasy
		this));
}

void game::draw_sprite_player() {

	HDC window = GetDC(player);
	HDC context_bitmap = CreateCompatibleDC(window);


	DeleteObject(SelectObject(context_bitmap, player_sprite_bitmap));

	BitBlt(window, 0, 0, player_size.x, player_size.y, context_bitmap, player_animation * 50, 0, SRCCOPY);

	DeleteDC(context_bitmap);
	ReleaseDC(player, window);

}
void game::draw_sprite_enemy() {


	HDC window = GetDC(enemies[0][0]); //////////////
	HDC context_bitmap = CreateCompatibleDC(window);
	HBITMAP oldBitmap = reinterpret_cast<HBITMAP>(SelectObject(context_bitmap, enemy_sprite_bitmap));




	for (int i = 0; i < ENEM_NB_ROW; i++) {
		for (int j = 0; j < ENEM_NB_COL; j++) {
			auto enemy_ct = GetDC(enemies[i][j]);
			BitBlt(enemy_ct, 0, 0, enemy_size.x, enemy_size.y, context_bitmap, enemy_animation * enemy_size.x, 0, SRCCOPY);

			ReleaseDC(enemies[i][j], enemy_ct);
		}
	}

	ReleaseDC(enemies[0][0], window);
	SelectObject(context_bitmap, oldBitmap);
	DeleteDC(context_bitmap);

	

}


void game::on_command(WPARAM wparam) {

	switch (LOWORD(wparam)) {
	case ID_SIZE_SMALL:
		size = { 600,400 };
		set_new_pos();

		draw_and_calc_overlay();
		return;
	case ID_SIZE_MEDIUM:
		size = { 800,600 };
		set_new_pos();

		draw_and_calc_overlay();///////
		return;
	case ID_SIZE_LARGE:
		size = { 1000,800 };
		set_new_pos();

		draw_and_calc_overlay();
		return;
	case ID_BACKGROUND_SOLID:
		ChooseColor(&choose_color);
		DeleteObject(m_background);
		m_background = CreateSolidBrush(choose_color.rgbResult);
		color = choose_color.rgbResult;

		draw_and_calc_overlay();
		return;
	case ID_BACKGROUND_IMAGE:
		GetOpenFileName(&open_file);

		main_background = (HBITMAP)LoadImage(NULL, file_name, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

		GetObject(main_background, sizeof(BITMAP), &bitmap_info);

		draw_and_calc_overlay();

		return;
	case ID_NEWGAME:

		score = 0;
		endgame = 0;
		for (int i = 0; i < ENEM_NB_ROW; i++) {
			for (int j = 0; j < ENEM_NB_COL; j++) {
				enemies_pos[i][j] = { start_enemies.x + j * (enemy_size.x + ENEM_MARG),   start_enemies.y + i * (enemy_size.y + ENEM_MARG) };
				if (enemies[i][j] != NULL)
					DestroyWindow(enemies[i][j]);


				enemies[i][j] = CreateWindowExW(
				0,
				L"STATIC",
				nullptr, ///  brak nazwy klasy sprawia ze okno powstaje z puli gotowych   czyli basic wyglad
				WS_CHILD | WS_VISIBLE | SS_CENTER,
				/// CHILD czyli nie wychodzi poza ramke?,       WS_VISIBLE okno poajawi sie bez show window     
				// CENTER  wyrownanie tekstu w oknie poziomo  do centrum
				enemies_pos[i][j].x, enemies_pos[i][j].y,// pozycja
				enemy_size.x, // rozmiar poziomo
				enemy_size.y, // rozmair pionowo
				m_main,
				nullptr,
				m_instance, /// klasa obslugujaca polecenia to tez glowna
				nullptr);
				

			}
		}



		draw_and_calc_overlay();

		return;
	case ID_IMAGE_FILL:
		CheckMenuRadioItem(menu, ID_IMAGE_CENTER, ID_IMAGE_FIT, ID_IMAGE_FILL, MF_CHECKED);
		image_type = Fill;

		draw_and_calc_overlay();
		return;

	case ID_IMAGE_CENTER:
		CheckMenuRadioItem(menu, ID_IMAGE_CENTER, ID_IMAGE_FIT, ID_IMAGE_CENTER, MF_CHECKED);
		image_type = Center;

		draw_and_calc_overlay();
		return;

	case ID_IMAGE_FIT:
		CheckMenuRadioItem(menu, ID_IMAGE_CENTER, ID_IMAGE_FIT, ID_IMAGE_FIT, MF_CHECKED);
		image_type = Fit;

		draw_and_calc_overlay();
		return;

	case ID_IMAGE_TILE:
		CheckMenuRadioItem(menu, ID_IMAGE_CENTER, ID_IMAGE_FIT, ID_IMAGE_TILE, MF_CHECKED);
		image_type = Tile;

		draw_and_calc_overlay();
		return;

	case ID_GAME_SAVE:
		save();
		return;

	case ID_GAME_LOAD:
		load();
		return;

	case ID_GAME_END:
		on_game_end();
		return;
	}
}

void game::set_new_pos() {

	left = (m_screen_size.x - size.x) / 2;
	top = (m_screen_size.y - size.y) / 2;


	player_pos = { (size.x - player_size.x) / 2, size.y - 2 * player_size.y - 20 };

	start_enemies = { (size.x - enemy_size.x) / 2 - (ENEM_NB_COL / 2) * (enemy_size.x + ENEM_MARG) ,  enemy_size.y - 20 };

	MoveWindow(m_main, left, top, size.x, size.y, true);
	MoveWindow(player, player_pos.x, player_pos.y, player_size.x, player_size.y, true);

	for (int i = 0; i < ENEM_NB_ROW; i++) {
		for (int j = 0; j < ENEM_NB_COL; j++) {
			enemies_pos[i][j] = { start_enemies.x + j * (enemy_size.x + ENEM_MARG),   start_enemies.y + i * (enemy_size.y + ENEM_MARG) };
			MoveWindow(enemies[i][j], enemies_pos[i][j].x, enemies_pos[i][j].y, enemy_size.x, enemy_size.y, true);
		}
	}

	RECT rect;
	GetClientRect(m_main, &rect);
	actual_size.x = rect.right - rect.left;
	actual_size.y = rect.bottom - rect.top;

}

void game::draw_and_calc_overlay() {

	calc_image_pos();
	HDC main_context = GetDC(m_main);
	update_overlay(main_context);
}

void game::update_overlay(HDC main_context) {

	RECT clientRect;
	GetClientRect(m_main, &clientRect);

	FillRect(main_context, &clientRect, m_background);



	HDC context_bitmap = CreateCompatibleDC(main_context);

	DeleteObject(SelectObject(context_bitmap, main_background));

	POINT pos = { 0,0 };
	float ratio_x, ratio_y,ratio;

	int new_size_x, new_size_y;



	if (main_background != NULL) {
		switch (image_type) {
		case Center:
			if (center_image_pos.x < 0 && center_image_pos.y < 0)
				BitBlt(main_context, 0, 0, actual_size.x, actual_size.y, context_bitmap, abs(center_image_pos.x), abs(center_image_pos.y), SRCCOPY);
			if (center_image_pos.x < 0 )
				BitBlt(main_context, 0, center_image_pos.y, actual_size.x, actual_size.y, context_bitmap, abs(center_image_pos.x), 0, SRCCOPY);
			if (center_image_pos.y < 0)
				BitBlt(main_context, center_image_pos.x, 0, actual_size.x, actual_size.y, context_bitmap, 0, abs(center_image_pos.y), SRCCOPY);

			BitBlt(main_context, center_image_pos.x, center_image_pos.y,  actual_size.x ,actual_size.y , context_bitmap, 0, 0, SRCCOPY);
			break;
		case Tile:
			while (pos.x < actual_size.x) {
				while (pos.y < actual_size.y) {
					BitBlt(main_context, pos.x, pos.y, actual_size.x - pos.x, actual_size.y - pos.y, context_bitmap, 0, 0, SRCCOPY);
					pos.y += bitmap_info.bmHeight;
				}
				pos.y = 0;
				pos.x += bitmap_info.bmWidth;
			}
			break;
		case Fill:
			StretchBlt(main_context, 0, 0, actual_size.x, actual_size.y, context_bitmap,
				0, 0, bitmap_info.bmWidth, bitmap_info.bmHeight, SRCCOPY);
			break;
		case Fit:
			ratio_x = (float)bitmap_info.bmWidth / (float)actual_size.x;
			ratio_y = (float)bitmap_info.bmHeight / (float)actual_size.y;
			if (ratio_x > ratio_y) 
				ratio = ratio_x;
			else
				ratio = ratio_y;


			new_size_x = bitmap_info.bmWidth / ratio;
			new_size_y = bitmap_info.bmHeight / ratio;

			StretchBlt(main_context, (actual_size.x - new_size_x) / 2, (actual_size.y - new_size_y) / 2, new_size_x, new_size_y, context_bitmap,
				0, 0, bitmap_info.bmWidth, bitmap_info.bmHeight, SRCCOPY);

		}
	}

	DeleteDC(context_bitmap);
	ReleaseDC(player, main_context);

	draw_sprite_player();
	draw_sprite_enemy();

	draw_score();
}

void game::calc_image_pos() {

	RECT rect;
	GetClientRect(m_main, &rect);
	actual_size.x = rect.right - rect.left;
	actual_size.y = rect.bottom - rect.top;

	center_image_pos = {
		(actual_size.x - (int)bitmap_info.bmWidth) / 2,
		(actual_size.y - (int)bitmap_info.bmHeight) / 2
	};
}

void game::draw_score() {
	HDC main_context = GetDC(m_main);
	if (score == ENEM_NB_COL * ENEM_NB_ROW && endgame == 0) {
		on_game_end();
		endgame = 1;
		return;
	}

	wchar_t buffer[10];
	swprintf(buffer, 10, L"%d", score);
	TextOut(main_context, 50, 10, (LPCWSTR)buffer, wcslen(buffer));


	ReleaseDC(m_main,main_context);
}

void game::save() {
	
	wchar_t buffer[256];
	HRESULT hr;
	BOOL success = TRUE;



	StringCchPrintf(buffer, 256, L"%d,%d,%d", GetRValue(color), GetGValue(color), GetBValue(color));
	WritePrivateProfileString(L"Settings", L"Color", buffer, iniFilePath);

	StringCchPrintf(buffer, 256, L"%ld,%ld", size.x, size.y);
	WritePrivateProfileString(L"Settings", L"Size", buffer, iniFilePath);



	WritePrivateProfileString(L"Settings", L"FileName", file_name, iniFilePath);



	StringCchPrintf(buffer, 256, L"%d", static_cast<int>(image_type));
	WritePrivateProfileString(L"Settings", L"ImageType", buffer, iniFilePath);
	


	WritePrivateProfileString(L"Settings", L"PlayerName", name, iniFilePath);


}

void game::load() {
	
	wchar_t buffer[256];
	BOOL success = TRUE;



	GetPrivateProfileString(L"Settings", L"Color", L"0,0,0", buffer, 256, iniFilePath);
	int r, g, b;
	swscanf_s(buffer, L"%d,%d,%d", &r, &g, &b);
	color = RGB(r, g, b);
	m_background = CreateSolidBrush(color);


	GetPrivateProfileString(L"Settings", L"Size", L"0,0", buffer, 256, iniFilePath);
	swscanf_s(buffer, L"%ld,%ld", &size.x, &size.y);
			



	GetPrivateProfileString(L"Settings", L"FileName", L"", file_name, MAX_PATH, iniFilePath);
	main_background = (HBITMAP)LoadImage(NULL, file_name, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	GetObject(main_background, sizeof(BITMAP), &bitmap_info);


	GetPrivateProfileString(L"Settings", L"ImageType", L"0", buffer, 256, iniFilePath);
	image_type = (ImageType)(_wtoi(buffer));

	switch (image_type) {
	case 0:
		PostMessage(m_main, WM_COMMAND, ID_IMAGE_CENTER, 0);
		break;
	case 1:
		PostMessage(m_main, WM_COMMAND, ID_IMAGE_FILL, 0);
		break;
	case 2:
		PostMessage(m_main, WM_COMMAND, ID_IMAGE_TILE, 0);
		break;
	case 3:
		PostMessage(m_main, WM_COMMAND, ID_IMAGE_FIT, 0);
		break;

	}


	GetPrivateProfileString(L"Settings", L"PlayerName", L"", name, 100, iniFilePath);
	
	set_new_pos();
	draw_and_calc_overlay();
	
}

void game::on_game_end() {
	create_and_show_end_window();
}

void game::create_and_show_end_window() {

	end_window = CreateDialog(m_instance, MAKEINTRESOURCE(ID_DIALOG), m_main, end_window_proc);
	HWND hScore = GetDlgItem(end_window, ID_SCORE);
	if (hScore)
	{
		wchar_t score_text[100];
		swprintf(score_text, 100, L"Score: %d", score); 
		SetWindowText(hScore, score_text);  
	}

	HWND hname = GetDlgItem(end_window, ID_NAME);
	if (hname)
	{
		wchar_t name_text[100];
		swprintf(name_text, 100, L"Hello %s", name);  
		SetWindowText(hname, name_text); 
	}


	ShowWindow(end_window, SW_SHOW);
}

INT_PTR game::end_window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam){
	HWND main;
	 switch (message)
     {
     case WM_INITDIALOG :
          return TRUE ;
          
		 
     case WM_COMMAND :
          switch (LOWORD (wparam))
          {
          case IDOK :
			main = GetParent(window);
			 PostMessage(main, WM_NAME, (WPARAM)window, 0);

             return TRUE ;
		  case IDCANCEL:
			  EndDialog(window, 0);
			  return TRUE;
          }
          break ;
     }
     return FALSE ;
}
