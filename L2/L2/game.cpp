#include "game.h"


std::wstring const game::s_class_name{ L"Space Invaders" };

bool game::register_class() {// rejestrowanie klasy czyli zapisanie co bedzie obslugiwac okienko 
	WNDCLASSEXW desc{}; // stworzenie struktury opisujacej klase do rejestracji

	if (GetClassInfoExW(m_instance, s_class_name.c_str(),
		&desc) != 0) return true; /// sprawdzenie czy klasa istnieje  


	desc = { .cbSize = sizeof(WNDCLASSEXW), // smieszne zabezpieczenie		!!!
	.lpfnWndProc = window_proc_static, // statyczna funkcja obslugi wiadomosci funkcja	!!!
	.hInstance = m_instance,     // instantcja klasy					!!!

	.hCursor = LoadCursorW(nullptr, L"IDC_ARROW"), // ladowanie kursora strzalka
	.hbrBackground = m_background, // tlo okienka   cos moze sie psuc jak sie tego nie ustawi

	.lpszMenuName = MAKEINTRESOURCEW(ID_MAINMENU), 
			//jesli menu jes dodane to tak sie je podlacza po id tu nie jest dodane

		.lpszClassName = s_class_name.c_str() }; // nazwa klasy wazne		!!!

	return RegisterClassExW(&desc) != 0; // zarejestrowanie klasy (0 jak sie nie powiodlo)
}

HWND game::create_window()
{

	HWND window = CreateWindowExW(
		WS_EX_TOPMOST | WS_EX_LAYERED,   // dodatkowe style   scrol bar,    overllaping .... 
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


	enemy_pos = { (size.x - enemy_size.x) / 2 ,  enemy_size.y - 20 };
	enemy = CreateWindowExW(
		0,
		L"STATIC",
		nullptr, ///  brak nazwy klasy sprawia ze okno powstaje z puli gotowych   czyli basic wyglad
		WS_CHILD | WS_VISIBLE | SS_CENTER,
		/// CHILD czyli nie wychodzi poza ramke?,       WS_VISIBLE okno poajawi sie bez show window     
		// CENTER  wyrownanie tekstu w oknie poziomo  do centrum
		enemy_pos.x, enemy_pos.y,// pozycja
		enemy_size.x, // rozmiar poziomo
		enemy_size.y, // rozmair pionowo
		window,
		nullptr,
		m_instance, /// klasa obslugujaca polecenia to tez glowna
		nullptr);

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
		draw_overlay((HDC)wparam);

		return 1;
	case WM_PAINT:

		return DefWindowProcW(window, message, wparam, lparam);
		

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
	enemy_brush{ CreateSolidBrush(RGB(70, 70, 255)) }, player_brush{ CreateSolidBrush(RGB(255, 0, 0)) },
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
}

int game::run(int show_command) // show zommand tez arg z maina jak ma sie stworzyc okno
{

	menu = GetMenu(m_main);
	ShowWindow(m_main, show_command); /// pokazanie okna powikeszenie i gueess
	SetTimer(m_main, s_timer, 1000, nullptr); /// 1000 ms

	SetTimer(m_main, bullet_timer, 15, nullptr); /// 1000 ms
	SetTimer(m_main, player_sprite_timer, 100, nullptr); /// 1000 ms

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

INT_PTR game::on_colorstatic(LPARAM lparam) {
	if ((HWND)lparam == enemy)
		return reinterpret_cast<INT_PTR>(enemy_brush);
	else if((HWND)lparam == player)
		return reinterpret_cast<INT_PTR>(player_brush);
	else
		return reinterpret_cast<INT_PTR>(bullet_brush);
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
		draw_sprite_player();
		return;
	case enemy_sprite_timer:
		draw_sprite_enemy();
		return;

	}

}

void game::move_enemy() {
	if (0 == enemy_status) {
		enemy_pos.x -= 25;
		MoveWindow(enemy, enemy_pos.x, enemy_pos.y, enemy_size.x, enemy_size.y, true);
		enemy_status = 1;
	}
	else if (1 == enemy_status) {

		enemy_pos.x += 25;
		MoveWindow(enemy, enemy_pos.x, enemy_pos.y, enemy_size.x, enemy_size.y, true);
		enemy_status = 0;
	}
}

void game::move_bullets() {

	auto it_pos = positions.begin();
	for (auto it_bullets = bullets.begin(); it_bullets != bullets.end(); ++it_bullets, ++it_pos) {
		(*it_pos).y -= offset;
		if ((*it_pos).y <= 0) {
			DestroyWindow(*it_bullets);
			continue;
		}
		MoveWindow(*it_bullets, (*it_pos).x,(*it_pos).y, bullet_size.x, bullet_size.y, true);
	}
}

void game::create_bullet() {
	positions.push_back({ player_pos.x + player_size.x / 2, player_pos.y - bullet_size.y });
	bullets.push_back(CreateWindowExW(
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

	player_animation++;
	player_animation %= 3;
	BitBlt(window, 0, 0, player_size.x, player_size.y, context_bitmap, player_animation * 50, 0, SRCCOPY);

	DeleteDC(context_bitmap);
	ReleaseDC(player, window);

}
void game::draw_sprite_enemy() {
	HDC window = GetDC(enemy);
	HDC context_bitmap = CreateCompatibleDC(window);

	HBITMAP oldBitmap = reinterpret_cast<HBITMAP>(SelectObject(context_bitmap, enemy_sprite_bitmap));

	enemy_animation++;
	enemy_animation %= 4;
	BitBlt(window, 0, 0, enemy_size.x, enemy_size.y, context_bitmap, enemy_animation * 50, 0, SRCCOPY);

	SelectObject(context_bitmap, oldBitmap);
	DeleteDC(context_bitmap);  
	ReleaseDC(enemy, window);
}


void game::on_command(WPARAM wparam) {

	switch (LOWORD(wparam)) {
	case ID_SIZE_SMALL:
		size = { 400,300 };
		calc_new_pos();
		return;
	case ID_SIZE_MEDIUM:
		size = { 800,600 };
		calc_new_pos();
		return;
	case ID_SIZE_LARGE:
		size = { 1000,800 };
		calc_new_pos();
		return;
	case ID_BACKGROUND_SOLID:
		ChooseColor(&choose_color);
		DeleteObject(m_background);
		m_background = CreateSolidBrush(choose_color.rgbResult);

		background_color();
		background_image();
		return;
	case ID_BACKGROUND_IMAGE:
		GetOpenFileName(&open_file);

		background_image();

		return;
	case ID_NEWGAME:
		return;
	case ID_IMAGE_FILL:
		CheckMenuRadioItem(menu, ID_IMAGE_CENTER, ID_IMAGE_FIT, ID_IMAGE_FILL, MF_CHECKED);
		image_type = Fill;
		return;

	case ID_IMAGE_CENTER:
		CheckMenuRadioItem(menu, ID_IMAGE_CENTER, ID_IMAGE_FIT, ID_IMAGE_CENTER, MF_CHECKED);
		image_type = Center;
		return;

	case ID_IMAGE_FIT:
		CheckMenuRadioItem(menu, ID_IMAGE_CENTER, ID_IMAGE_FIT, ID_IMAGE_FIT, MF_CHECKED);
		image_type = Fit;
		return;

	case ID_IMAGE_TILE:
		CheckMenuRadioItem(menu, ID_IMAGE_CENTER, ID_IMAGE_FIT, ID_IMAGE_TILE, MF_CHECKED);
		image_type = Tile;
		return;


	}
}

void game::calc_new_pos() {

	left = (m_screen_size.x - size.x) / 2;
	top = (m_screen_size.y - size.y) / 2;


	player_pos = { (size.x - player_size.x) / 2, size.y - 2 * player_size.y - 20 };

	enemy_pos = { (size.x - enemy_size.x) / 2 ,  enemy_size.y - 20 };

	MoveWindow(m_main, left, top, size.x, size.y, true);
	MoveWindow(player, player_pos.x, player_pos.y, player_size.x, player_size.y, true);
	MoveWindow(enemy, enemy_pos.x, enemy_pos.y, enemy_size.x, enemy_size.y, true);

}

void game::background_color() {
	
	HDC main_context = GetDC(m_main);
	
	RECT clientRect;
	GetClientRect(m_main, &clientRect);

	FillRect(main_context, &clientRect, m_background);

	ReleaseDC(m_main, main_context);
}


void game::background_image() {

	
	main_background = (HBITMAP)LoadImage(NULL, file_name, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	HDC window = GetDC(m_main);
	HDC context_bitmap = CreateCompatibleDC(window);

	DeleteObject(SelectObject(context_bitmap, main_background));


	BitBlt(window, 0, player_pos.y, size.x, size.y, context_bitmap, 0, 0, SRCCOPY);//////////

	DeleteDC(context_bitmap);
	ReleaseDC(player, window);
}

void game::draw_overlay(HDC main_context) {

	RECT clientRect;
	GetClientRect(m_main, &clientRect);

	FillRect(main_context, &clientRect, m_background);


	main_background = (HBITMAP)LoadImage(NULL, file_name, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	HDC context_bitmap = CreateCompatibleDC(main_context);

	DeleteObject(SelectObject(context_bitmap, main_background));


	BitBlt(main_context, 0, player_pos.y, size.x, size.y, context_bitmap, 0, 0, SRCCOPY);////////////

	DeleteDC(context_bitmap);
	ReleaseDC(player, main_context);

	draw_sprite_player();
	draw_sprite_enemy();
}