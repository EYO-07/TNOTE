#include "functions.h"

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

// -- ENTRY POINT
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    //LoadLibrary(TEXT("Msftedit.dll"));

    // Register the window class.
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.
    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST,             // Extended window styles.
        CLASS_NAME,                 // Window class
        L"TNOTE",          // Window text
        WS_OVERLAPPEDWINDOW,        // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    // Set window transparency (optional)
    SetLayeredWindowAttributes(hwnd, 0, (BYTE)200, LWA_ALPHA); // Fully opaque

    ShowWindow(hwnd, nCmdShow);

    // Create Edit Control
    
    hwndEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE, L"EDIT", NULL,
        WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
        0, 0, CW_USEDEFAULT, CW_USEDEFAULT,
        hwnd, NULL, hInstance, NULL
    );
    
    /*
    // Rich Edit
    hwndEdit = CreateWindowEx(
        0, MSFTEDIT_CLASS, NULL,
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
        0, 0, CW_USEDEFAULT, CW_USEDEFAULT,
        hwnd, NULL, hInstance, NULL
    );
    */

    // Garante que o foco seja aplicado logo após criar o controle
    SetFocus(hwndEdit);

    // Subclass the Edit Control
    oldEditProc = (WNDPROC)SetWindowLongPtr(hwndEdit, GWLP_WNDPROC, (LONG_PTR)EditProc);

    // Adjust the size of the Edit Control immediately
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    SetWindowPos(hwndEdit, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER | SWP_NOACTIVATE);

    // Create background brush
    hbrBkgnd = CreateSolidBrush(RGB(0, 0, 0)); // Black background

    // Create a font (Consolas, Bold)
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
    //SetupRichEdit(hwndEdit);

    std::wstring initial_text(L"// TNOTE : Translucent Notepad\r\n// (Press F1 to run): @{font:13} @{color:white} @{400,400} @compact @shrink \r\n// (Put @ to run): -compact load exit -shrink rshrink -rshrink {eval:1+2}\r\n// alt+arrow_key moves, ctrl+s saves, F1 execute commands");
    SetWindowText(hwndEdit, initial_text.c_str());

    // Run the message loop.
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Clean up
    DeleteObject(hbrBkgnd);
    DeleteObject(hFont);

    return 0;
}

// -- 
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        // Add a menu for file operations
    {
        AddMenu(hwnd); // Add the menu
    }
    break;

    case WM_SETFOCUS:
        SetFocus(hwndEdit); // Certifica-se de que o foco vai para o EDIT
        return 0;

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

    case WM_SIZE:
        // Resize the Edit Control to fit the window
    {
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);
        SetWindowPos(hwndEdit, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER | SWP_NOACTIVATE);
    }
    break;

    case WM_ACTIVATE:
        // Change window transparency based on focus
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            SetLayeredWindowAttributes(hwnd, 0, (BYTE)50, LWA_ALPHA); // Set transparency to 100 when inactive
			if( isShrinkMode ){
				int width = getWidth(hwnd);
				int height = getHeight(hwnd);
				ResizeWindow(hwnd, width, height/5);
			}
			if (is_rShrinkMode) {
				int width = getWidth(hwnd);
				int height = getHeight(hwnd);
				ResizeWindow(hwnd, width/5, height);
			}
        }
        else
        {
            SetLayeredWindowAttributes(hwnd, 0, (BYTE)200, LWA_ALPHA); // Set transparency to 200 when active
			if( isShrinkMode ){
				int width = getWidth(hwnd);
				int height = getHeight(hwnd);
				ResizeWindow(hwnd, width, 5*height);
			}
			if (is_rShrinkMode) {
				int width = getWidth(hwnd);
				int height = getHeight(hwnd);
				ResizeWindow(hwnd, 5*width, height);
			}
        }
        break;

    case WM_CTLCOLOREDIT:
    {
        HDC hdcEdit = (HDC)wParam;
        SetTextColor(hdcEdit, textColor); // Yellow text
        SetBkColor(hdcEdit, RGB(0, 0, 0)); // Black background
        return (LRESULT)hbrBkgnd;
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

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

        // Check if Ctrl+S is pressed
        if ((GetKeyState(VK_CONTROL) & 0x8000) && (wParam == 'S'))
        {
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

    if (uMsg == WM_KEYDOWN && wParam == VK_F1) // F1 pressed
    {
        CheckForCommands();
        return 0;
    }

    // Call the original window procedure for default handling
    return CallWindowProc(oldEditProc, hwnd, uMsg, wParam, lParam);
}















