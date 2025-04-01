#include "functions.h"

// --
std::vector<std::wstring> latex_content;
std::wstring latex_headers =  L"\\documentclass[10pt,a4paper]{article}\r\n\\usepackage{blindtext}\r\n\\usepackage[utf8]{inputenc}\r\n\\usepackage{amsmath}\r\n\\usepackage{amsfonts}\r\n\\usepackage{amssymb}\r\n\\usepackage{graphicx}\r\n\\usepackage{mathrsfs}\r\n\\setlength\\topmargin{0in}\r\n\\setlength\\headheight{0in}\r\n\\setlength\\headsep{0in}\r\n\\setlength\\textheight{8.9in}\r\n\\setlength\\textwidth{6.5in}\r\n\\setlength\\oddsidemargin{0in}\r\n\\setlength\\evensidemargin{0in}\r\n\\usepackage{listings}\r\n";

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
			KillTimer(GetParent(hwndEdit), TIMER_ID);
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
			full_width = width;
			full_height = height;
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
		if (text.find(L"@dropdown") != std::wstring::npos) isDropDown = true;
		if (text.find(L"@-dropdown") != std::wstring::npos) isDropDown = false;
		if (text.find(L"@dropright") != std::wstring::npos) isDropRight = true;
		if (text.find(L"@-dropright") != std::wstring::npos) isDropRight = false;
        
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
	if(isCompactMode) return ;
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
	if(!isCompactMode) return ;
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
	//if(!isCompactMode) return ;
    SetWindowPos(hwnd, NULL, 0, 0, newWidth, newHeight, SWP_NOMOVE | SWP_NOZORDER);
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

void MoveWindow(HWND hWnd, int dx, int dy) {
    if (hWnd == nullptr) {
        return; // Verifica se o identificador da janela é válido
    }

    // Obtém as coordenadas atuais da janela
    RECT rect;
    if (GetWindowRect(hWnd, &rect)) {
        // Calcula as novas coordenadas
        int newX = rect.left + dx;
        int newY = rect.top + dy;

        // Move a janela para as novas coordenadas
        SetWindowPos(hWnd, nullptr, newX, newY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }
}

void CheckForF5Commands(HWND hwndEdit){
	// -- 
	if (hwndEdit == nullptr) return ;
	latex_content.clear();
	LoadLatexHeaders();
	latex_content.push_back(latex_headers);
	latex_content.push_back(L"\r\n\\begin{document}\r\n");
	//
	int length = GetWindowTextLength(hwndEdit);
	if (length == 0) return;    
	wchar_t* buffer = new wchar_t[length + 1];
	GetWindowText(hwndEdit, buffer, length + 1);
	std::wstring text(buffer);
	// -- 
	std::wregex regexLatex(L"\\[(tex[^>]*)>([^<]*)<end\\]");
	auto in = std::wsregex_iterator(text.begin(), text.end(), regexLatex);
    auto fi = std::wsregex_iterator(); // Iterador final padrão
	int count = 0;
	for (auto it = in; it != fi; ++it){
		std::wsmatch matchesLatex = *it;
		// -- 
		if ( matchesLatex[1].str()==L"tex" )
		{
			latex_content.push_back( matchesLatex[2].str() );
			latex_content.push_back( L"\r\n\\hspace{\\baselineskip}\r\n" );
			count++;
		}
		else if ( matchesLatex[1].str()==L"tex:itemize" || matchesLatex[1].str()==L"tex:item" )
		{
			latex_content.push_back(L"\\begin{itemize}\r\n");
			latex_content.push_back( matchesLatex[2].str() );
			latex_content.push_back(L"\r\n\\end{itemize}\r\n");
			latex_content.push_back( L"\r\n\\hspace{\\baselineskip}\r\n" );
			count++;
		}
		else if ( matchesLatex[1].str()==L"tex:verbatim" || matchesLatex[1].str()==L"tex:verb" )
		{
			latex_content.push_back(L"\\begin{verbatim}\r\n");
			latex_content.push_back( matchesLatex[2].str() );
			latex_content.push_back(L"\r\n\\end{verbatim}\r\n");
			latex_content.push_back( L"\r\n\\hspace{\\baselineskip}\r\n" );
			count++;
		}
		else if ( matchesLatex[1].str()==L"tex:enumerate" || matchesLatex[1].str()==L"tex:enum" )
		{
			latex_content.push_back(L"\\begin{enumerate}\r\n");
			latex_content.push_back( matchesLatex[2].str() );
			latex_content.push_back(L"\r\n\\end{enumerate}\r\n");
			latex_content.push_back( L"\r\n\\hspace{\\baselineskip}\r\n" );
			count++;
		}
		else if ( matchesLatex[1].str()==L"tex:math" || matchesLatex[1].str()==L"tex:$$" )
		{
			latex_content.push_back(L"$$ ");
			latex_content.push_back( matchesLatex[2].str() );
			latex_content.push_back(L" $$");
			latex_content.push_back( L"\r\n\\hspace{\\baselineskip}\r\n" );
			count++;
		}
		else if ( matchesLatex[1].str()==L"tex:code" )
		{
			latex_content.push_back(L"\\begin{lstlisting}");
			latex_content.push_back( matchesLatex[2].str() );
			latex_content.push_back(L"\r\n\\end{lstlisting}\r\n");
			latex_content.push_back( L"\r\n\\hspace{\\baselineskip}\r\n" );
			count++;
		}
	}
	latex_content.push_back( L"\r\n\\hspace{\\baselineskip}\r\n" );
	latex_content.push_back(L"\\end{document}\r\n");
	delete[] buffer;
	if (count==0) latex_content.clear();
}

void CreateLatex(HWND hwnd) {
	if ( currentFileName.empty()) return ;
    // Verifica se há conteúdo no vetor latex_content
    if (latex_content.empty()) return;
	if (latex_content.size()==0) return;
    // Nome fixo para o arquivo LaTeX
    std::wstring fileName = currentFileName+L".tex";

    // Cria o arquivo no diretório atual
    HANDLE hFile = CreateFile(
        fileName.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    // Verifica se o arquivo foi criado com sucesso
    if (hFile == INVALID_HANDLE_VALUE) return;

    // Converte o conteúdo do vetor latex_content em uma única string
    std::wstringstream ss;
    for (const auto& it : latex_content) ss << it;
    std::wstring end_tex = ss.str();

    // Converte de std::wstring (UTF-16) para std::string (UTF-8)
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string utf8Content = converter.to_bytes(end_tex);

    // Escreve o conteúdo convertido (UTF-8) no arquivo
    DWORD dwWritten;
    WriteFile(hFile, utf8Content.c_str(), (DWORD)utf8Content.size(), &dwWritten, NULL);

    // Fecha o handle do arquivo
    CloseHandle(hFile);
}

void CompileLatex() {
	if (!ExistsCommand(L"pdflatex") ){ 
		MessageBox(nullptr, L"can't find pdflatex, please install miktek", L"Error", MB_OK | MB_ICONERROR);
		return ;
	}
	if (latex_content.empty()) return;
	if (latex_content.size()==0) return;
	if ( currentFileName.empty()) return ;
    // Comando para compilar o arquivo .tex
	//std::wstring filename_path(GetDirectory(currentFileName));
    std::wstring filename = currentFileName+L".tex";
	if(!FileExists(filename)) return ;
	//filename=filename_path + filename;
    std::wstring command = L"pdflatex -interaction=nonstopmode \""+filename+L"\"";
    // Configuração do processo
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi = { 0 };
    // Criação do processo
    if (CreateProcess(
            nullptr, // Nome do aplicativo (nullptr porque está no comando)
            &command[0], // Comando a ser executado
            nullptr, // Segurança do processo
            nullptr, // Segurança da thread
            FALSE, // Herança de handles
            0, // Flags de criação
            nullptr, // Ambiente
            nullptr, // Diretório de trabalho
            &si, // Informações de inicialização
            &pi // Informações do processo
        )) {
        // Aguarda o término do processo
        //WaitForSingleObject(pi.hProcess, INFINITE);
        // Fecha os handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        // Caso ocorra um erro
        MessageBox(nullptr, L"Compile Error", L"Erro", MB_OK | MB_ICONERROR);
    }
}

std::wstring GetDirectory(const std::wstring& filePath) {
    wchar_t buffer[MAX_PATH];
    wcscpy_s(buffer, filePath.c_str());

    // Remove o nome do arquivo para obter apenas o diretório
    PathRemoveFileSpec(buffer); // Função da biblioteca Shlwapi
    return std::wstring(buffer);
}

void OpenPDF(){
	if (latex_content.empty()) return;
	if (latex_content.size()==0) return;
	if ( currentFileName.empty()) return ;
	std::wstring filename = currentFileName+L".pdf";
	std::wstring command = filename;
	if(!FileExists(filename)) return ;
	// Configuração do processo
    // Usa ShellExecute para abrir o arquivo com o programa padrão
    HINSTANCE result = ShellExecute(
        NULL,        // Janela do proprietário (NULL para nenhum)
        L"open",     // Ação (abrir o arquivo)
        filename.c_str(), // Caminho completo do arquivo
        NULL,        // Parâmetros (não necessário para abrir arquivos)
        NULL,        // Diretório de trabalho (NULL para o atual)
        SW_SHOWNORMAL // Mostra a janela do aplicativo
    );
    // Verifica se houve erro
    if ((INT_PTR)result <= 32) {
        std::wcerr << "Error while opening the file: " << result << std::endl;
    }
}

bool FileExists(const std::wstring& filePath) {
    DWORD fileAttributes = GetFileAttributes(filePath.c_str());
    return (fileAttributes != INVALID_FILE_ATTRIBUTES &&
            !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY)); // Certifica que não é um diretório
}

void LoadLatexHeaders() {
	if (currentFileName.empty()) return ;
	std::wstring filename = L"headers.tex";
	if( !FileExists( filename ) )
	{
		// -- create and use default headers
		// Cria o arquivo no diretório atual
		HANDLE hFile = CreateFile(
			filename.c_str(),
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		// Verifica se o arquivo foi criado com sucesso
		if (hFile == INVALID_HANDLE_VALUE) return;

		// Converte de std::wstring (UTF-16) para std::string (UTF-8)
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		std::string utf8Content = converter.to_bytes(latex_headers);
		DWORD dwWritten;
		WriteFile(hFile, utf8Content.c_str(), (DWORD)utf8Content.size(), &dwWritten, NULL);
		// Fecha o handle do arquivo
		CloseHandle(hFile);
	} 
	else 
	{
		// -- load the headers from file
		latex_headers = LoadTextFileUTF8(filename);
	}
	
}

std::wstring LoadTextFile(std::wstring filename) {
    // Abre o arquivo para leitura
    HANDLE hFile = CreateFileW(
        filename.c_str(),          // Nome do arquivo
        GENERIC_READ,              // Acesso de leitura
        FILE_SHARE_READ,           // Permitir leitura compartilhada
        NULL,                      // Sem segurança especial
        OPEN_EXISTING,             // Abrir arquivo existente
        FILE_ATTRIBUTE_NORMAL,     // Atributos normais
        NULL                       // Sem template para criação
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Falha ao abrir o arquivo.");
    }

    // Obtem o tamanho do arquivo
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        throw std::runtime_error("Falha ao obter o tamanho do arquivo.");
    }

    // Lê o conteúdo do arquivo
    wchar_t* buffer = new wchar_t[fileSize / sizeof(wchar_t) + 1]; // Aloca buffer
    DWORD bytesRead;
    if (!ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
        delete[] buffer;
        CloseHandle(hFile);
        throw std::runtime_error("Falha ao ler o arquivo.");
    }
    buffer[fileSize / sizeof(wchar_t)] = L'\0'; // Garantir que a string está terminada

    // Fecha o handle do arquivo
    CloseHandle(hFile);

    // Converte o buffer para std::wstring
    std::wstring content(buffer);
    delete[] buffer;

    return content;
}

std::wstring LoadTextFileUTF8(std::wstring filename) {
    // Abre o arquivo para leitura
    HANDLE hFile = CreateFileW(
        filename.c_str(),          // Nome do arquivo
        GENERIC_READ,              // Acesso de leitura
        FILE_SHARE_READ,           // Permitir leitura compartilhada
        NULL,                      // Sem segurança especial
        OPEN_EXISTING,             // Abrir arquivo existente
        FILE_ATTRIBUTE_NORMAL,     // Atributos normais
        NULL                       // Sem template para criação
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Falha ao abrir o arquivo.");
    }

    // Obtém o tamanho do arquivo
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        throw std::runtime_error("Falha ao obter o tamanho do arquivo.");
    }

    // Lê o conteúdo do arquivo em um buffer
    char* buffer = new char[fileSize + 1]; // Aloca buffer para bytes
    DWORD bytesRead;
    if (!ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
        delete[] buffer;
        CloseHandle(hFile);
        throw std::runtime_error("Falha ao ler o arquivo.");
    }
    buffer[fileSize] = '\0'; // Garantir que o buffer está terminado

    // Fecha o handle do arquivo
    CloseHandle(hFile);

    // Converte de UTF-8 para UTF-16 (std::wstring)
    int wideCharSize = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, NULL, 0);
    if (wideCharSize == 0) {
        delete[] buffer;
        throw std::runtime_error("Falha ao converter para UTF-16.");
    }

    wchar_t* wideBuffer = new wchar_t[wideCharSize];
    MultiByteToWideChar(CP_UTF8, 0, buffer, -1, wideBuffer, wideCharSize);

    std::wstring content(wideBuffer);
    delete[] buffer;
    delete[] wideBuffer;

    return content;
}

// Função para verificar se o comando existe
bool ExistsCommand(const std::wstring& command) {
    // Variável para armazenar o caminho completo do comando encontrado
    wchar_t buffer[MAX_PATH];

    // Tenta localizar o comando no PATH do sistema
    DWORD result = SearchPathW(
        NULL,          // Usa as pastas do PATH do sistema
        command.c_str(), // Nome do comando
        L".exe",       // Extensão padrão a ser considerada (executáveis)
        MAX_PATH,      // Tamanho do buffer
        buffer,        // Buffer para armazenar o caminho completo do comando
        NULL           // Parte restante do nome (opcional, não usada aqui)
    );

    // Se o resultado for diferente de 0, o comando foi encontrado
    return result != 0;
}









