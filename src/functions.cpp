#include "functions.h"

// --
void ChangeFontSize(int newSize)
{
    // Create a new font with the specified size
    HFONT newFont = CreateFont(
        newSize,                    // Height
        0,                          // Width
        0,                          // Escapement
        0,                          // Orientation
        FW_BOLD,                    // Weight (bold)
        FALSE,                      // Italic
        FALSE,                      // Underline
        FALSE,                      // StrikeOut
        ANSI_CHARSET,               // CharSet
        OUT_DEFAULT_PRECIS,         // OutPrecision
        CLIP_DEFAULT_PRECIS,        // ClipPrecision
        DEFAULT_QUALITY,            // Quality
        FIXED_PITCH | FF_MODERN,    // PitchAndFamily
        L"Consolas");               // FaceName

    // Apply the new font to the edit control
    SendMessage(hwndEdit, WM_SETFONT, (WPARAM)newFont, TRUE);

    // Delete the old font and update the global font variable
    DeleteObject(hFont);
    hFont = newFont;
}
void CheckForCommands()
{
    int length = GetWindowTextLength(hwndEdit);
    if (length > 0)
    {
        wchar_t* buffer = new wchar_t[length + 1];
        GetWindowText(hwndEdit, buffer, length + 1);

        std::wstring text(buffer);

        // Check for the @exit command anywhere in the text
        if (text.find(L"@exit") != std::wstring::npos)
        {
            PostQuitMessage(0);
			return;
        }
		
		// Check for the @load command to load a file
        if (text.find(L"@load") != std::wstring::npos)
        {
            LoadFile(GetParent(hwndEdit));
			return;
        }

        // Check for the @{fonte:X} command to change font size
        std::wregex regexFont(L"\\@\\{font:(\\d+)\\}");
        std::wsmatch matchesFont;
        if (std::regex_search(text, matchesFont, regexFont))
        {
            int newSize = std::stoi(matchesFont[1].str());
            ChangeFontSize(newSize);
        }

        // Check for the @{color:R,G,B} command to change font color
        std::wregex regexColorRGB(L"\\@\\{color:(\\d+),(\\d+),(\\d+)\\}");
        std::wsmatch matchesColorRGB;
        if (std::regex_search(text, matchesColorRGB, regexColorRGB))
        {
            int r = std::stoi(matchesColorRGB[1].str());
            int g = std::stoi(matchesColorRGB[2].str());
            int b = std::stoi(matchesColorRGB[3].str());
            textColor = RGB(r, g, b);

            // Force a redraw of the edit control
            RedrawWindow(hwndEdit, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
        }

        // Check for the @{color:X} command to change font color using common names
        std::wregex regexColorName(L"\\@\\{color:([a-zA-Z]+)\\}");
        std::wsmatch matchesColorName;
        if (std::regex_search(text, matchesColorName, regexColorName))
        {
            std::wstring colorName = matchesColorName[1].str();
            if (colorMap.find(colorName) != colorMap.end())
            {
                textColor = colorMap[colorName];

                // Force a redraw of the edit control
                RedrawWindow(hwndEdit, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
            }
        }

        // Check for the @{X,Y} command to change window size
        std::wregex regexSize(L"\\@\\{(\\d+),(\\d+)\\}");
        std::wsmatch matchesSize;
        if (std::regex_search(text, matchesSize, regexSize))
        {
            int width = std::stoi(matchesSize[1].str());
            int height = std::stoi(matchesSize[2].str());
            SetWindowPos(GetParent(hwndEdit), NULL, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE | SWP_NOOWNERZORDER);
        }

        // Check for the @compact command to make the main window borderless and hide scrollbars
        if (text.find(L"@compact") != std::wstring::npos) compact(hwndEdit, GetParent(hwndEdit));
        if (text.find(L"@-compact") != std::wstring::npos) full(hwndEdit, GetParent(hwndEdit));
		
        // Verificar o comando @{eval:X}
        std::wregex regexEval(L"\\@\\{eval:([^\\}]+)\\}");
        std::wsmatch matchesEval;
        if (std::regex_search(text, matchesEval, regexEval))
        {
            try
            {
                std::wstring expression = matchesEval[1].str();
                double result = EvaluateExpression(expression);

                // Inserir o resultado no texto após o comando
                std::wstring resultText = L" = " + std::to_wstring(result);
                size_t pos = matchesEval.position() + matchesEval.length();
                text.insert(pos, resultText);
                SetWindowText(hwndEdit, text.c_str());
            }
            catch (const std::exception& e)
            {
                MessageBox(NULL, L"Syntax Error", L"Error", MB_OK | MB_ICONERROR);
            }
        }

		if (text.find(L"@shrink") != std::wstring::npos) isShrinkMode = true;
		if (text.find(L"@-shrink") != std::wstring::npos) isShrinkMode = false;
		if (text.find(L"@rshrink") != std::wstring::npos) is_rShrinkMode = true;
		if (text.find(L"@-rshrink") != std::wstring::npos) is_rShrinkMode = false;
        
		delete[] buffer;
    }
}
std::wstring GetFileNameFromPath(const std::wstring& filePath)
{
    wchar_t fileName[_MAX_FNAME];
    wchar_t fileExt[_MAX_EXT];
    _wsplitpath_s(filePath.c_str(), NULL, 0, NULL, 0, fileName, _MAX_FNAME, fileExt, _MAX_EXT);

    return std::wstring(fileName) + std::wstring(fileExt);
}
void LoadFile(HWND hwnd)
{
    // Open Load File Dialog
    OPENFILENAME ofn;
    wchar_t szFile[260] = { 0 };
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Text Files\0*.TXT\0All Files\0*.*\0";
    ofn.lpstrDefExt = L"txt";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn))
    {
        HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD dwFileSize = GetFileSize(hFile, NULL);
            wchar_t* pszText = new wchar_t[dwFileSize / sizeof(wchar_t) + 1];

            DWORD dwRead;
            ReadFile(hFile, pszText, dwFileSize, &dwRead, NULL);
            pszText[dwFileSize / sizeof(wchar_t)] = L'\0'; // Null-terminate the string

            SetWindowText(hwndEdit, pszText);

            // Store the current file name and update the window title
            currentFileName = ofn.lpstrFile;
            std::wstring windowTitle = L"TNOTE (" + currentFileName + L")";
            SetWindowText(hwnd, windowTitle.c_str());

            CloseHandle(hFile);
            delete[] pszText;
        }
    }
}
void SaveFile(HWND hwnd)
{
    // Open Save File Dialog
    OPENFILENAME ofn;
    wchar_t szFile[260] = { 0 };
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;

    // If a file is loaded, suggest its name
    if (!currentFileName.empty())
    {
        std::wstring fileName = GetFileNameFromPath(currentFileName);
        wcsncpy_s(szFile, fileName.c_str(), _TRUNCATE);
    }

    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Text Files\0*.TXT\0All Files\0*.*\0";
    ofn.lpstrDefExt = L"txt";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn))
    {
        HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD dwTextLength = GetWindowTextLength(hwndEdit);
            wchar_t* pszText = new wchar_t[dwTextLength + 1];
            GetWindowText(hwndEdit, pszText, dwTextLength + 1);

            DWORD dwWritten;
            WriteFile(hFile, pszText, dwTextLength * sizeof(wchar_t), &dwWritten, NULL);

            // Store the current file name and update the window title
            currentFileName = ofn.lpstrFile;
            std::wstring windowTitle = L"TNOTE (" + currentFileName + L")";
            SetWindowText(hwnd, windowTitle.c_str());

            CloseHandle(hFile);
            delete[] pszText;
        }
    }
}
void AddMenu(HWND hwnd)
{
    HMENU hMenu = CreateMenu();
    HMENU hFileMenu = CreateMenu();
    AppendMenu(hFileMenu, MF_STRING, 1, L"Load");
    AppendMenu(hFileMenu, MF_STRING, 2, L"Save");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"File");
    SetMenu(hwnd, hMenu);
}
double EvaluateExpression(const std::wstring& expression)
{
    std::wistringstream stream(expression);
    std::stack<double> values;
    std::stack<wchar_t> operators;

    auto precedence = [](wchar_t op) -> int {
        if (op == '+' || op == '-') return 1;
        if (op == '*' || op == '/') return 2;
        return 0;
        };

    auto applyOperation = [](double a, double b, wchar_t op) -> double {
        switch (op)
        {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (b == 0.0) throw std::runtime_error("Divisão por zero");
            return a / b;
        default: throw std::runtime_error("Operador inválido");
        }
        };

    wchar_t token;
    while (stream >> token)
    {
        if (std::isdigit(token) || token == '.')
        {
            stream.putback(token);
            double value;
            stream >> value;
            values.push(value);
        }
        else if (token == '(')
        {
            operators.push(token);
        }
        else if (token == ')')
        {
            while (!operators.empty() && operators.top() != '(')
            {
                double b = values.top(); values.pop();
                double a = values.top(); values.pop();
                wchar_t op = operators.top(); operators.pop();
                values.push(applyOperation(a, b, op));
            }
            operators.pop(); // Remove '('
        }
        else
        {
            while (!operators.empty() && precedence(operators.top()) >= precedence(token))
            {
                double b = values.top(); values.pop();
                double a = values.top(); values.pop();
                wchar_t op = operators.top(); operators.pop();
                values.push(applyOperation(a, b, op));
            }
            operators.push(token);
        }
    }

    while (!operators.empty())
    {
        double b = values.top(); values.pop();
        double a = values.top(); values.pop();
        wchar_t op = operators.top(); operators.pop();
        values.push(applyOperation(a, b, op));
    }

    return values.top();
}
void compact(HWND child, HWND parent){
    // Remove the menu
    SetMenu(parent, NULL);

    // Set the main window to borderless style
    SetWindowLong(parent, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    SetWindowPos(child, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);

    // Remove scrollbars from the edit control
    SetWindowLong(child, GWL_STYLE, GetWindowLong(child, GWL_STYLE) & ~WS_HSCROLL & ~WS_VSCROLL);
    SetWindowPos(child, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    isCompactMode = true; // Atualiza o estado
}
void full(HWND child, HWND parent){
    // Restore the menu
    AddMenu(parent); // Add the menu

    // Restore the original window style
    SetWindowLong(parent, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
    SetWindowPos(parent, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);

    // Restore scrollbars in the edit control
    SetWindowLong(child, GWL_STYLE, GetWindowLong(child, GWL_STYLE) | WS_HSCROLL | WS_VSCROLL);
    SetWindowPos(child, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    isCompactMode = false; // Atualiza o estado
}

void ResizeWindow(HWND hwnd, int newWidth, int newHeight) {
    SetWindowPos(hwnd, NULL, 0, 0, newWidth, newHeight, 
                 SWP_NOMOVE | SWP_NOZORDER);
}

int getWidth(HWND hwnd){
    RECT rect;
    if (GetClientRect(hwnd, &rect)) return rect.right - rect.left;
	return 100;
}

int getHeight(HWND hwnd){
    RECT rect;
	if (GetClientRect(hwnd, &rect)) return rect.bottom - rect.top;
	return 100;
}

/*
void SetupRichEdit(HWND hwndEdit)
{
    // Define o fundo preto
    SendMessage(hwndEdit, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0, 0, 0)); // Fundo preto

    // Configura a cor de texto padrão como amarelo
    CHARFORMAT2 cfDefault;
    ZeroMemory(&cfDefault, sizeof(CHARFORMAT2));
    cfDefault.cbSize = sizeof(CHARFORMAT2);
    cfDefault.dwMask = CFM_COLOR;
    cfDefault.crTextColor = RGB(255, 255, 0); // Amarelo
    SendMessage(hwndEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cfDefault);
}
void HighlightKeywords(HWND hwndEdit)
{
    // Palavras-chave que devem ser destacadas
    std::vector<std::wstring> keywords = { L"@exit", L"@load", L"@save", L"@compact", L"@-compact" };

    CHARRANGE charRange;
    CHARFORMAT2 charFormat;

    std::wstring text;
    int length = GetWindowTextLength(hwndEdit);
    text.resize(length);
    GetWindowText(hwndEdit, &text[0], length + 1);

    for (const auto& keyword : keywords)
    {
        size_t pos = text.find(keyword);
        while (pos != std::wstring::npos)
        {
            // Configura o intervalo de caracteres (start e end)
            charRange.cpMin = pos;
            charRange.cpMax = pos + keyword.length();
            SendMessage(hwndEdit, EM_EXSETSEL, 0, (LPARAM)&charRange);

            // Configura o formato de texto
            memset(&charFormat, 0, sizeof(CHARFORMAT2));
            charFormat.cbSize = sizeof(CHARFORMAT2);
            charFormat.dwMask = CFM_COLOR | CFM_BOLD;
            charFormat.crTextColor = RGB(255, 0, 0); // Cor vermelha
            charFormat.dwEffects = CFE_BOLD;

            // Aplica o formato
            SendMessage(hwndEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&charFormat);

            // Continua procurando a próxima palavra-chave
            pos = text.find(keyword, pos + keyword.length());
        }
    }

    // Remove a seleção no final
    charRange.cpMin = -1;
    charRange.cpMax = -1;
    SendMessage(hwndEdit, EM_EXSETSEL, 0, (LPARAM)&charRange);
}

*/




