#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "Window.h"

namespace help {
	extern void* createFunctionGateway(void* function, byte* bytes, int numBytes);
	extern BOOL(__stdcall* PostMessageW_Gateway)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	extern void PostMessageW_Gateway_init(void);
    extern std::vector<std::string> splitSentenceIntoWords(std::string sentence);
    extern std::string getWindowStringText(HWND hwnd);
    extern void fetchWindows(std::vector<Window>& list, int filter = 0);
	extern int stovkc(std::string key);
	extern std::vector<int> sstovkc(std::string key);
}