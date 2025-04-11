#include "functions.h"
#include "resource.h"

// Forward declaration of the Window Procedure functions
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Global variables
HWND hwndEdit;
HBRUSH hbrBkgnd;
HFONT hFont;
WNDPROC oldEditProc; // Original Edit Control Window Procedure
COLORREF textColor = RGB(255, 255, 0); // Default yellow text
std::wstring currentFileName;
std::unordered_map<std::wstring, COLORREF> colorMap = {
    {L"red", RGB(255, 0, 0)},
    {L"green", RGB(0, 255, 0)},
    {L"blue", RGB(0, 0, 255)},
    {L"cyan", RGB(0, 255, 255)},
    {L"magenta", RGB(255, 0, 255)},
    {L"yellow", RGB(255, 255, 0)},
    {L"black", RGB(0, 0, 0)},
    {L"white", RGB(255, 255, 255)},
    {L"gray", RGB(128, 128, 128)}
};
bool isCompactMode = false;
bool isShrinkMode = false;
bool is_rShrinkMode = false;
int full_width, full_height ;
const int TIMER_ID = 1;
const int STEP_SIZE = 10;
bool isDropDown = false;
bool isDropRight = false;
bool isGlobalHotkeyActive = false;
// int character_limit = 32767;

// -- ENTRY POINT
// Keyboard Input ~ EditProc() || ... | % WM_SYSKEYDOWN | % WM_KEYDOWN || % VK_F1 | % VK_F5 || CheckForF5Commands() | CreateLatex() | CompileLatex() | OpenPDF() 
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// -- global hotkeys
    if (!RegisterHotKey(NULL, 1, MOD_WIN|MOD_ALT, VK_LEFT)) std::cout << "Can't register the hotkey \n";
	if (!RegisterHotKey(NULL, 2, MOD_WIN|MOD_ALT, VK_RIGHT)) std::cout << "Can't register the hotkey \n";
	if (!RegisterHotKey(NULL, 3, MOD_WIN|MOD_ALT, VK_UP)) std::cout << "Can't register the hotkey \n";
	if (!RegisterHotKey(NULL, 4, MOD_WIN|MOD_ALT, VK_DOWN)) std::cout << "Can't register the hotkey \n";
	if (!RegisterHotKey(NULL, 5, MOD_WIN|MOD_ALT, VK_PRIOR)) std::cout << "Can't register the hotkey \n"; // up
	if (!RegisterHotKey(NULL, 6, MOD_WIN|MOD_ALT, VK_NEXT)) std::cout << "Can't register the hotkey \n"; // down
	if (!RegisterHotKey(NULL, 7, MOD_WIN|MOD_ALT, VK_HOME)) std::cout << "Can't register the hotkey \n";
	if (!RegisterHotKey(NULL, 8, MOD_WIN|MOD_ALT, VK_END)) std::cout << "Can't register the hotkey \n";
	
	// -- 
	const wchar_t CLASS_NAME[] = L"Sample Window Class";
    WNDCLASS wc = { }; {	
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = hInstance;
		wc.lpszClassName = CLASS_NAME;
        wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
		RegisterClass(&wc);
	}
	// -- 
    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST,             // Extended window styles.
        CLASS_NAME,                 // Window class
        L"TNOTE",          // Window text
        WS_OVERLAPPEDWINDOW,        // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // Size and position
        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );
    if (hwnd == NULL) return 0;
    SetLayeredWindowAttributes(hwnd, 0, (BYTE)200, LWA_ALPHA); 
    ShowWindow(hwnd, nCmdShow);
	full_width = getWidth(hwnd);
	full_height = getHeight(hwnd);
    // Create Edit Control
    hwndEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE, L"EDIT", NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
        0, 0, CW_USEDEFAULT, CW_USEDEFAULT,
        hwnd, NULL, hInstance, NULL
    );
    SetFocus(hwndEdit);
    oldEditProc = (WNDPROC)SetWindowLongPtr(hwndEdit, GWLP_WNDPROC, (LONG_PTR)EditProc);
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    SetWindowPos(hwndEdit, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER | SWP_NOACTIVATE);
    // -- 
    hbrBkgnd = CreateSolidBrush(RGB(0, 0, 0)); // Black background
    hFont = CreateFont(
        14,                        // Height
        0,                         // Width
        0,                         // Escapement
        0,                         // Orientation
        FW_BOLD,                   // Weight (bold)
        FALSE,                     // Italic
        FALSE,                     // Underline
        FALSE,                     // StrikeOut
        ANSI_CHARSET,              // CharSet
        OUT_DEFAULT_PRECIS,        // OutPrecision
        CLIP_DEFAULT_PRECIS,       // ClipPrecision
        DEFAULT_QUALITY,           // Quality
        FIXED_PITCH | FF_MODERN,   // PitchAndFamily
        L"Consolas");              // FaceName
    SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hFont, TRUE); // Set the font for the edit control
	SendMessage(hwndEdit, EM_FMTLINES, TRUE, 0);

	// Remover limite fixo de caracteres
	//SendMessage(hwndEdit, EM_SETLIMITTEXT, character_limit, 0);
    // 
    // -- 
    std::wstring initial_text(L"// TNOTE : Translucent Notepad\r\n// (Press F1 to run): @{font:13} @{color:white} @{700,400} @compact @shrink \r\n// (Put @ to run): hotkeys -hotkeys -compact load exit -shrink rshrink -rshrink {eval:1+2} -dropdown dropdown -dropright dropright \r\n// alt+arrow_key moves, ctrl+s saves, F1 execute commands");
    SetWindowText(hwndEdit, initial_text.c_str());
    // Run the message loop.
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
		// -- Global Hotkeys
		if(isGlobalHotkeyActive==true){
			HWND currentHWND = GetForegroundWindow();
			if (currentHWND){
				if( msg.message == WM_HOTKEY ){
					switch (msg.wParam) {
						case 1: // WIN|ALT + Left
							// Lógica para mover a janela para esquerda
							MoveWindow(currentHWND, -10,0);
							break;
						case 2: // WIN|ALT + Right
							// Lógica para mover a janela para direita
							MoveWindow(currentHWND, 10,0);
							break;
						case 3: // WIN|ALT + up
							// Lógica para mover a janela para cima
							MoveWindow(currentHWND, 0,-10);
							break;
						case 4: // WIN|ALT + down
							// Lógica para mover a janela para baixo 
							MoveWindow(currentHWND, 0,10);
							break;
						case 5: // WIN|ALT + Page Up
							RatioResizeWindow(currentHWND, 1.05);
							break;
						case 6: // WIN|ALT + Page Down
							RatioResizeWindow(currentHWND, 0.95);
							break;	
						case 7:
							VerticalResize(currentHWND);
							break; 
						case 8:
							HorizontalResize(currentHWND);
							break; 
					}
				}
			}
		}
    }
    
	// Clean up
    DeleteObject(hbrBkgnd);
    DeleteObject(hFont);
	// Libera as hotkeys ao sair
    UnregisterHotKey(NULL, 1);
    UnregisterHotKey(NULL, 2);
	UnregisterHotKey(NULL, 3);
	UnregisterHotKey(NULL, 4);
	UnregisterHotKey(NULL, 5);
	UnregisterHotKey(NULL, 6);
	UnregisterHotKey(NULL, 7);
	UnregisterHotKey(NULL, 8);
    return 0;
}

// -- 
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE: // Add a menu for file operations
    {
        AddMenu(hwnd); // Add the menu
    }
    break;

    case WM_SETFOCUS: {
        SetFocus(hwndEdit); // Certifica-se de que o foco vai para o EDIT
        return 0;
	}
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case 1: // Load menu item clicked
            LoadFile(hwnd);
            break;
        case 2: // Save menu item clicked
            SaveFile(hwnd);
            break;
        }
        break;

    case WM_SIZE: // Resize the Edit Control to fit the window
    {
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);
        SetWindowPos(hwndEdit, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER | SWP_NOACTIVATE);
    }
    break;
	case WM_TIMER: 
	{
		bool isExpanding, is_rExpanding;
		int current_width = getWidth(hwnd);
		int current_height = getHeight(hwnd);
		isExpanding = ( isShrinkMode && (current_height<full_height) );
		is_rExpanding = ( is_rShrinkMode && (current_width<full_width) );
		if( isExpanding || is_rExpanding ){
			if(isExpanding && isCompactMode){ 
				if( isDropDown ){ ResizeWindow(hwnd, current_width, min(current_height+STEP_SIZE, full_height)); } else {
					int changed_height = min(current_height+STEP_SIZE, full_height);
					int dy = changed_height - current_height;
					MoveWindow(hwnd, 0, -dy);
					ResizeWindow(hwnd, current_width, changed_height);
				}
			}
			if(is_rExpanding && isCompactMode){ 
				if( isDropRight ){ ResizeWindow(hwnd, min(current_width+STEP_SIZE, full_width), getHeight(hwnd)); } else {
					int changed_width = min(current_width+STEP_SIZE, full_width);
					int dx = changed_width - current_width;
					MoveWindow(hwnd, -dx, 0);
					ResizeWindow(hwnd, changed_width, getHeight(hwnd));
				}
			}
		}
		else {
			KillTimer(hwnd, TIMER_ID);
		}
	}
    case WM_ACTIVATE:
	{
        // Change window transparency based on focus
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            SetLayeredWindowAttributes(hwnd, 0, (BYTE)50, LWA_ALPHA); // Set transparency to 100 when inactive
			KillTimer(hwnd, TIMER_ID);
			if( isShrinkMode && isCompactMode){ 
				if ( isDropDown ) {	ResizeWindow(hwnd, getWidth(hwnd), getHeight(hwnd)/8); } else {
					int changed_height = floor( getHeight(hwnd)/8 );
					int dy = getHeight(hwnd) - changed_height;
					MoveWindow(hwnd, 0, dy);
					ResizeWindow(hwnd, getWidth(hwnd), changed_height); 
				}
			}
			if( is_rShrinkMode && isCompactMode){ 
				if ( isDropRight ) { ResizeWindow(hwnd, getWidth(hwnd)/10, getHeight(hwnd)); } else {
					int changed_width = floor( getWidth(hwnd)/10 );
					int dx = getWidth(hwnd) - changed_width;
					MoveWindow(hwnd, dx, 0);
					ResizeWindow(hwnd, changed_width, getHeight(hwnd));
				}
			}
        }
        else
        {
            SetLayeredWindowAttributes(hwnd, 0, (BYTE)200, LWA_ALPHA); // Set transparency to 200 when active
			if( isShrinkMode || is_rShrinkMode ) SetTimer(hwnd, TIMER_ID,10,NULL);
        }
        break;
	}
    case WM_CTLCOLOREDIT:
    {
        HDC hdcEdit = (HDC)wParam;
        SetTextColor(hdcEdit, textColor); // Yellow text
        SetBkColor(hdcEdit, RGB(0, 0, 0)); // Black background
        return (LRESULT)hbrBkgnd;
    }
    break;

    case WM_DESTROY: 
	{
		KillTimer(hwnd, TIMER_ID);
        PostQuitMessage(0);
        return 0;
	}
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
    }
    return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// EditProc() || % WM_SYSKEYDOWN or WM_KEYDOWN || % alt || % up | % down | % left | % right
LRESULT CALLBACK EditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_LBUTTONDBLCLK)
    {
        HWND hwndMain = GetParent(hwnd); // Obter o identificador da janela principal

        if (isCompactMode) { full(hwnd, hwndMain); } else
        {
            compact(hwnd, hwndMain);
        }
    }

    if (uMsg == WM_SYSKEYDOWN || uMsg == WM_KEYDOWN)
    {
        // Check if Alt key is pressed and handle arrow keys to move the window
        if (wParam == VK_MENU || (lParam & (1 << 29))) // Check if Alt key is pressed
        {            
			if (!isGlobalHotkeyActive){
				HWND hwndMain = GetParent(hwnd); // Get the handle to the main window
				RECT rect;
				GetWindowRect(hwndMain, &rect);
				switch (wParam)
				{
				case VK_UP: // Alt + Up Arrow
					SetWindowPos(hwndMain, NULL, rect.left, rect.top - 10, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
					return 0;
				case VK_DOWN: // Alt + Down Arrow
					SetWindowPos(hwndMain, NULL, rect.left, rect.top + 10, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
					return 0;
				case VK_LEFT: // Alt + Left Arrow
					SetWindowPos(hwndMain, NULL, rect.left - 10, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
					return 0;
				case VK_RIGHT: // Alt + Right Arrow
					SetWindowPos(hwndMain, NULL, rect.left + 10, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
					return 0;
				}
			}
        }

        // Check if Ctrl+S is pressed
        if ((GetKeyState(VK_CONTROL) & 0x8000) && (wParam == 'S'))
        {
			UpdateCharLim(hwnd);
            if (!currentFileName.empty())
            {
                // Save the file directly if currentFileName has a file name
                HANDLE hFile = CreateFile(currentFileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE)
                {
                    DWORD dwTextLength = GetWindowTextLength(hwndEdit);
                    wchar_t* pszText = new wchar_t[dwTextLength + 1];
                    GetWindowText(hwndEdit, pszText, dwTextLength + 1);

                    DWORD dwWritten;
                    WriteFile(hFile, pszText, dwTextLength * sizeof(wchar_t), &dwWritten, NULL);

                    CloseHandle(hFile);
                    delete[] pszText;
                }
            }
            else
            {
                // If currentFileName is empty, show the Save File Dialog
                SaveFile(GetParent(hwnd));
            }
            return 0;
        }

    }

    if (uMsg == WM_KEYDOWN ) // F1, F5 pressed
    {
		if(wParam == VK_F1){
			CheckForCommands();
			return 0;
		} 
		else if (wParam == VK_F5){
			CheckForF5Commands(hwnd);
			CreateLatex( GetParent( hwnd) );
			CompileLatex();
			OpenPDF();
			return 0;
		}
    }

    // Call the original window procedure for default handling
    return CallWindowProc(oldEditProc, hwnd, uMsg, wParam, lParam);
}















