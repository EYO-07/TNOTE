#pragma once

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <windows.h>
#include <string>
#include <regex>
#include <unordered_map>
#include <commdlg.h> 
#include <sstream>
#include <stack>
#include <cctype>
#include <cmath>
#include <vector>
#include <Shlwapi.h>
#include <codecvt>
#include <iostream>
#include <stdexcept>
// #include <richedit.h>

// -- functions
void ChangeFontSize(int newSize);
void CheckForCommands();
std::wstring GetFileNameFromPath(const std::wstring& filePath);
void LoadFile(HWND hwnd);
void SaveFile(HWND hwnd);
void AddMenu(HWND hwnd);
double EvaluateExpression(const std::wstring& expression);
void compact(HWND child, HWND parent);
void full(HWND child, HWND parent);
// void SetupRichEdit(HWND hwndEdit);
// void HighlightKeywords(HWND hwndEdit);
void ResizeWindow(HWND hwnd, int newWidth, int newHeight);
int getWidth(HWND hwnd);
int getHeight(HWND hwnd);
void MoveWindow(HWND hWnd, int dx, int dy);
void CheckForF5Commands(HWND);
void CreateLatex(HWND);
void CompileLatex();
std::wstring GetDirectory(const std::wstring& filePath);
void OpenPDF();
bool FileExists(const std::wstring& filePath);
void LoadLatexHeaders();
std::wstring LoadTextFile(std::wstring filename);
std::wstring LoadTextFileUTF8(std::wstring filename);
bool ExistsCommand(const std::wstring& command);
void RatioResizeWindow(HWND hwnd, double ratio);
void VerticalResize(HWND hwnd); 
void HorizontalResize(HWND hwnd); 
void UpdateCharLim(HWND hwndEdit);

// -- global external vars
extern HWND hwndEdit;
extern COLORREF textColor;
extern std::wstring currentFileName;
extern HFONT hFont;
extern std::unordered_map<std::wstring, COLORREF> colorMap;
extern bool isCompactMode;
extern bool isShrinkMode;
extern bool is_rShrinkMode;
extern const int TIMER_ID;
extern int full_width;
extern int full_height;
extern bool isDropDown;
extern bool isDropRight;
extern std::vector<std::wstring> latex_content; 
extern std::wstring latex_headers;
extern bool isGlobalHotkeyActive;
//extern int character_limit;

// --

#endif // FUNCTIONS_H
