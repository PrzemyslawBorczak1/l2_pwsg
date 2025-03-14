#include "game.h"
#include <stdexcept>
#include<dwmapi.h>

std::wstring const game::s_class_name{ L"Space Invaders" };

bool game::register_class() {// rejestrowanie klasy czyli zapisanie co bedzie obslugiwac okienko 
	WNDCLASSEXW desc{}; // stworzenie struktury opisujacej klase do rejestracji

	if (GetClassInfoExW(m_instance, s_class_name.c_str(),
		&desc) != 0) return true; /// sprawdzenie czy klasa istnieje  


	desc = { .cbSize = sizeof(WNDCLASSEXW), // smieszne zabezpieczenie		!!!
	.lpfnWndProc = window_proc_static, // statyczna funkcja obslugi wiadomosci funkcja	!!!
	.hInstance = m_instance,     // instantcja klasy					!!!

	.hCursor = LoadCursorW(nullptr, L"IDC_ARROW"), // ladowanie kursora strzalka
	.hbrBackground = CreateSolidBrush(RGB(250, 247, 238)), // tlo okienka   cos moze sie psuc jak sie tego nie ustawi

		//.lpszMenuName = MAKEINTRESOURCEW(ID_MAINMENU), 
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


	enemy_pos = { (size.x - enemy_size.x) / 2 ,  100 };
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

	player_pos = { (size.x - player_size.x) / 2, size.y - 100 };
	player = CreateWindowExW(
		0,
		L"STATIC",
		nullptr, ///  brak nazwy klasy sprawia ze okno powstaje z puli gotowych   czyli basic wyglad
		WS_CHILD | WS_VISIBLE | SS_CENTER,
		/// CHILD czyli nie wychodzi poza ramke?,       WS_VISIBLE okno poajawi sie bez show window     
		// CENTER  wyrownanie tekstu w oknie poziomo  do centrum
		player_pos.x, player_pos.y,// pozycja
		enemy_size.x, // rozmiar poziomo
		enemy_size.y, // rozmair pionowo
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
	enemy_brush{ CreateSolidBrush(RGB(70, 70, 255)) }, player_brush{ CreateSolidBrush(RGB(255, 0, 0)) }

{
	left = (m_screen_size.x - 800) / 2;
	top = (m_screen_size.y - 600) / 2;
	register_class();
	m_main = create_window();
}

int game::run(int show_command) // show zommand tez arg z maina jak ma sie stworzyc okno
{
	ShowWindow(m_main, show_command); /// pokazanie okna powikeszenie i gueess
	SetTimer(m_main, s_timer, 1000, nullptr); /// 1000 ms

	SetTimer(m_main, bullet_timer, 15, nullptr); /// 1000 ms

	MSG msg{};
	BOOL result = TRUE;
	while ((result = GetMessageW(&msg, nullptr, 0, 0)) != 0) /// loop message    peek gory 
		//arg: strukturka gdzie ma wpisac, okno od ktorego ma brac,  min i max eartosci wiadomosci
	{
		if (result == -1) // blad
			return EXIT_FAILURE;

		TranslateMessage(&msg); // dla inutu z klawiatury 
		DispatchMessageW(&msg); /// odbieranie wiadomosci

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
	else
		return reinterpret_cast<INT_PTR>(player_brush);
}

void game::on_keydown(WPARAM wparam) {
	if (VK_LEFT == wparam) {
		if (player_pos.x < 35)
			return;
		player_pos.x -= 25;
		MoveWindow(player, player_pos.x, player_pos.y, player_size.x, player_size.y, true);
	}
	else if (VK_RIGHT == wparam) {
		if (player_pos.x > 700 - 25) 
			return;
		player_pos.x += 25;
		MoveWindow(player, player_pos.x, player_pos.y, player_size.x, player_size.y, true);
	}
}

void game::on_timer(WPARAM wparam) {
	if (s_timer == wparam)
		move_enemy();
	else if (bullet_timer == wparam)
		move_bullets();


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

}