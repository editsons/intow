#include "helpers.h"

namespace help {

    void* createFunctionGateway(void* function, byte* bytes, int numBytes) {
        byte* gateway = (byte*)VirtualAlloc(NULL, numBytes + 5, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (gateway == NULL) return NULL;

        for (int i = 0; i < numBytes; i++) {
            gateway[i] = bytes[i];
        }
        gateway[numBytes] = 0xe9;
        *(unsigned int*)&gateway[numBytes + 1] = ((byte*)(function)+numBytes) - &gateway[numBytes + 5];   // write jump offset
        return gateway;
    }

    typedef BOOL(__stdcall* PostMessageWT)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    PostMessageWT PostMessageW_Gateway = nullptr;

    void PostMessageW_Gateway_init(void) {
        byte bytes[] = { 0x8b, 0xff, 0x55, 0x8b, 0xec };   // original bytes of PostMessageW
        PostMessageW_Gateway = (PostMessageWT)createFunctionGateway(PostMessageW, bytes, 5);
    }

    std::vector<std::string> splitSentenceIntoWords(std::string sentence)
    {
        std::vector<std::string> words;
        std::string word = "";
        for (auto c : sentence)
        {
            if (c == ' ') {
                if (word == "") continue;
                words.push_back(word);
                word = "";
            }
            else {
                word += c;
            }
        }
        words.push_back(word);
        return words;
    }
    
    std::string getWindowStringText(HWND hwnd)
    {
        int len = GetWindowTextLength(hwnd) + 1;
        std::vector<wchar_t> buf(len);
        GetWindowText(hwnd, &buf[0], len);
        std::wstring wide = &buf[0];
        std::string s(wide.begin(), wide.end());
        return s;
    }
    
    struct fetchWindows_params {
        std::vector<Window>* list;
        int filter;
        bool isChildCall;
        std::string windowText;
        fetchWindows_params(std::vector<Window>* list, int filter, bool isChildCall, std::string windowText) {
            this->list = list;
            this->filter = filter;
            this->isChildCall = isChildCall;
            this->windowText = windowText;
        }
    };
    BOOL CALLBACK fetchWindows_proc(__in HWND hwnd, __in LPARAM param) {
        if (param == NULL) return FALSE;
        fetchWindows_params* params = (fetchWindows_params*)param;

        // filter windows
        if (params->filter > 0) {
            if (!IsWindowVisible(hwnd)) return TRUE; // visibility

            if (params->filter > 1) {
                if (getWindowStringText(hwnd).length() == 0) return TRUE;  // title length

                if (params->filter > 2) {
                    WINDOWINFO windowInfo;
                    windowInfo.cbSize = sizeof(WINDOWINFO);
                    if (GetWindowInfo(hwnd, &windowInfo) == 0) return TRUE;
                    if (windowInfo.cxWindowBorders <= 0 || windowInfo.cyWindowBorders <= 0) return TRUE; // border size
                }
            }
        }

        // setup window text (title)
        if (!params->isChildCall) params->windowText = getWindowStringText(hwnd);
        else params->windowText += "_child_" + getWindowStringText(hwnd);

        // add window to list
        params->list->push_back(Window(hwnd, params->windowText));

        // add child windows
        if (params->filter < 3) {
            if (GetWindow(hwnd, GW_CHILD)) {
                fetchWindows_params* paramsChilds = new fetchWindows_params(params->list, params->filter, true, params->windowText);
                EnumChildWindows(hwnd, fetchWindows_proc, (LPARAM)paramsChilds);
                delete paramsChilds;
            }
        }
        return TRUE;
    }
    void fetchWindows(std::vector<Window>& list, int filter) {
        fetchWindows_params* params = new fetchWindows_params(&list, filter, false, "");
        EnumWindows(fetchWindows_proc, (LPARAM)params);
        delete params;
    }

    int stovkc(std::string key) {
        std::unordered_map<std::string, int> g_vkCodeMap = {
            { "0", 0x30 },
            { "1", 0x31 },
            { "2", 0x32 },
            { "3", 0x33 },
            { "4", 0x34 },
            { "5", 0x35 },
            { "6", 0x36 },
            { "7", 0x37 },
            { "8", 0x38 },
            { "9", 0x39 },
            { "ALT", VK_MENU },
        };
        SHORT vkCode = -1;
        try { vkCode = g_vkCodeMap.at(key); }
        catch (...) {}
        return vkCode;
    }

    std::vector<int> sstovkc(std::string keys) {

        std::vector<int> result;
        std::string subString;
        bool subSearch = false;

        for (size_t i = 0; i < keys.length(); i++) {

            int vkCode = help::stovkc(std::string(1, keys[i]));

            if (subSearch) {
                if (keys[i] == '>') {
                    vkCode = help::stovkc(subString);
                    subSearch = false;
                }
                else if (keys[i] == ')') {
                    try { vkCode = std::stoi(subString, nullptr, 16); }
                    catch (...) { vkCode = -1; }
                    subSearch = false;
                }
                else {
                    subString += keys[i];
                    continue;
                }
            }

            if (keys[i] == '<' || keys[i] == '(') {
                subString = "";
                subSearch = true;
                continue;
            }

            result.push_back(vkCode);
        }

        return result;
    }
}