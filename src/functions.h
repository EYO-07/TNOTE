#pragma once

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <windows.h>
#include <string>
#include <regex>
#include <unordered_map>
#include <commdlg.h>  // For common dialogs (e.g., SaveFile dialog)

#include <sstream>
#include <stack>
#include <cctype>
#include <cmath>
// #include <richedit.h>

// Declarações das funções
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

extern HWND hwndEdit;
extern COLORREF textColor;
extern std::wstring currentFileName;
extern HFONT hFont;
extern std::unordered_map<std::wstring, COLORREF> colorMap;
extern bool isCompactMode;
extern bool isShrinkMode;
extern bool is_rShrinkMode;

#endif // FUNCTIONS_H
