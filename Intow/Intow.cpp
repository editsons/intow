#include <iostream>
#include <Windows.h>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <map>
#include "VirtualKeyCodeMap.h"


typedef BOOL(__stdcall* PostMessageWT)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
PostMessageWT PostMessageW_Gateway = NULL;


SHORT getVirtualKeyCode(std::string key) {
    SHORT vkCode = 0;
    try { vkCode = g_vkCodeMap.at(key); }
    catch (const std::out_of_range& oor) {}
    return vkCode;
}



struct Window {
    HWND handle;
    std::string name;

    Window(HWND handle, std::string name) {
        this->handle = handle;
        this->name = name;
    }
};

class Input {

private:
    Window window;
    std::vector<unsigned char> vkCodes;
    int delay1, delay2;
    bool isDestructing, isRunning;
    std::thread writeKeyThread, readKeyThread;
    SHORT toggleKey;
    bool toggleKeyHold;

public:
    Input(Window window, std::vector<unsigned char> vkCodes, int delay1, int delay2)
        :window(window)
    {
        this->vkCodes = vkCodes;
        this->delay1 = delay1;
        this->delay2 = delay2;
        this->isDestructing = false;
        this->isRunning = false;
        writeKeyThread = std::thread(&Input::thread_writeKey, this);
        readKeyThread = std::thread(&Input::thread_readKey, this);
        this->toggleKey = 0;
        this->toggleKeyHold = false;
    }

    ~Input() {
        stop();
        isDestructing = true;
        writeKeyThread.join();
        readKeyThread.join();
    }

    void start() {
        if (!isRunning) {
            isRunning = true;
        }
    }

    void stop() {
        if (isRunning) {
            isRunning = false;
        }
    }

    void thread_writeKey() {
        while (!isDestructing) {
            if(isRunning) {
                for (int i = 0; i < vkCodes.size(); i++) {
                    if (PostMessageW_Gateway(window.handle, WM_KEYDOWN, vkCodes[i], 1) == NULL) {
                        std::cout << "Error: Failed to send input \"" << std::string(vkCodes.begin(), vkCodes.end()) << "\" to window \"" << window.name << "\"." << std::endl;
                        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
                    }
                    else if (i < vkCodes.size() - 1) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(delay2));
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(delay1));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    void thread_readKey() {
        while (!isDestructing) {
            if (toggleKey != 0) {
                if (GetAsyncKeyState(toggleKey)) {
                    toggleKeyHold = true;
                }
                else if (toggleKeyHold) {
                    toggleKeyHold = false;
                    // key released
                    printf("Key released!\n");
                    if (isRunning) stop();
                    else start();
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    std::vector<unsigned char> getVirtualKeyCodes() {
        return vkCodes;
    }

    void setToggleKey(SHORT vkCode) {
        toggleKey = vkCode;
    }

    SHORT getToggleKey() {
        return toggleKey;
    }

    static std::vector<unsigned char> parseKeys(std::string keys) {

        std::vector<unsigned char> result;
        std::string subString;
        bool subSearch = false;

        for (int i = 0; i < keys.length(); i++) {

            if (subSearch) {
                if (keys[i] == '>') {
                    SHORT vkCode = getVirtualKeyCode(subString);
                    if (vkCode) { result.push_back(vkCode); }
                    else { result.clear(); break; }
                    subSearch = false;
                    continue;
                }
                subString += keys[i];
                continue;
            }

            if (keys[i] == '<') {
                subString = "";
                subSearch = true;
                continue;
            }

            SHORT vkCode = getVirtualKeyCode(std::string(1, keys[i]));
            if (vkCode) { result.push_back(vkCode); }
            else { result.clear(); break; }
        }

        return result;
    }
};


std::vector<Window> g_windowList;
std::vector<Input*> g_inputList;


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
std::vector<std::string> splitSentenceIntoWords(std::string sentence)
{
    std::vector<std::string> words;
    std::string word = "";
    for (auto c : sentence)
    {
        if (c == ' ') {
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
void fetchWindows(std::vector<Window>& list, int filter = 0) {
    fetchWindows_params* params = new fetchWindows_params(&list, filter, false, "");
    EnumWindows(fetchWindows_proc, (LPARAM)params);
    delete params;
}


void clearConsole() {
    system("cls");
    std::cout << "Use \"listWindows\" to fetch a list of all windows (\"listWindows2\" or \"listWindows3\" for less filtering)." << std::endl;
    std::cout << "Use \"listInputs\" to display all current running input threads." << std::endl;
    std::cout << "Use \"focus <windowId>\" to focus a window." << std::endl;
    std::cout << "Use \"new\" to create a new input thread." << std::endl;
    std::cout << "Use \"delete\" to delete an input thread." << std::endl;
    std::cout << std::endl;
}

void printWindowList(int filter = 0) {
    std::cout << "- - - - - - - - - - Window list: - - - - - - - - - - " << std::endl;
    std::cout << "ID                Description        " << std::endl;
    g_windowList.clear();
    fetchWindows(g_windowList, filter);
    for (int i = 0; i < g_windowList.size(); i++) {
        std::cout << i << ": " << g_windowList[i].name << std::endl;
    }
    std::cout << std::endl;
}


void cmd_clear() {
    clearConsole();
}
void cmd_listWindows() {
    clearConsole();
    printWindowList(3);
}
void cmd_listWindows2() {
    clearConsole();
    printWindowList(1);
}
void cmd_listWindows3() {
    clearConsole();
    printWindowList(0);
}
void cmd_listInputs() {
    // check if empty
    // ... TODO
    clearConsole();
    std::cout << "ID                Description        " << std::endl;
    for (int i = 0; i < g_inputList.size(); i++) {
        std::cout << i << ": " << std::string(g_inputList[i]->getVirtualKeyCodes().begin(), g_inputList[i]->getVirtualKeyCodes().end()) << std::endl;
    }
}
void cmd_focus(std::vector<std::string>& command) {
    if (command.size() > 1) {
        int windowId = std::stoi(command[1]);
        if (windowId >= 0 && windowId < g_windowList.size()) {
            SetForegroundWindow(g_windowList[windowId].handle);
            std::cout << "Window " << windowId << " successfully focused." << std::endl << std::endl;
        }
        else {
            std::cout << "Error: Invalid window id." << std::endl << std::endl;
        }
    }
    else {
        std::cout << "Usage: focus <windowId>" << std::endl << std::endl;
    }
}
void cmd_new(std::vector<std::string>& command) {
    if (command.size() >= 4) {
        int windowId = std::stoi(command[1]);
        if (windowId >= 0 && windowId < g_windowList.size()) {
            std::vector<unsigned char> virtualKeyCodes = Input::parseKeys(command[2]);
            if (virtualKeyCodes.size() > 0) {
                int delay2 = command.size() < 5 ? 1 : std::stoi(command[4]);
                g_inputList.push_back(new Input(g_windowList[windowId], virtualKeyCodes, std::stoi(command[3]), delay2));
                g_inputList.back()->start();
            }
            else {
                std::cout << "Error: Failed to parse keys." << std::endl << std::endl;
            }
        }
        else {
            std::cout << "Error: Invalid window id." << std::endl;
        }
    }
    else {
        std::cout << "Usage: new <windowId> <keys> <delay1_ms> [<delay2_ms>]" << std::endl << std::endl;
    }
}
void cmd_delete(std::vector<std::string>& command) {
    if (command.size() >= 2) {
        int windowId = std::stoi(command[1]);
        if (windowId >= 0 && windowId < g_windowList.size()) {
            g_inputList.erase(g_inputList.begin() + windowId);
        }
        else {
            std::cout << "Error: Invalid window id." << std::endl;
        }
    }
    else {
        std::cout << "Usage: delete <inputId>" << std::endl << std::endl;
    }
}
void cmd_setToggleKey(std::vector<std::string>& command) {
    if (command.size() > 2) {
        int inputId = std::stoi(command[1]);
        std::string key = command[2];
        if (inputId >= 0 && inputId < g_inputList.size()) {
            SHORT vkCode = getVirtualKeyCode(key);
            if (vkCode != 0) {
                g_inputList[inputId]->setToggleKey(vkCode);
            }
            else {
                std::cout << "Error: Invalid key. Enter \"listKeys\" for a list of available keys." << std::endl;
            }
        }
        else {
            std::cout << "Error: Invalid input id." << std::endl;
        }
    }
    else {
        std::cout << "Usage: setToggleKey <inputId> <key>" << std::endl << std::endl;
    }
}


void inputThread() {
    std::string input;
    do {
        std::getline(std::cin, input);
        std::vector<std::string> command = splitSentenceIntoWords(input);
        if (command.size() == 0) continue;
        for (auto& c : command[0]) c = toupper(c);

        if (command[0].compare("CLEAR") == 0) cmd_clear();
        else if (command[0].compare("LISTWINDOWS") == 0) cmd_listWindows();
        else if (command[0].compare("LISTWINDOWS2") == 0) cmd_listWindows2();
        else if (command[0].compare("LISTWINDOWS3") == 0) cmd_listWindows3();
        else if (command[0].compare("LISTINPUTS") == 0) cmd_listInputs();
        else if (command[0].compare("FOCUS") == 0) cmd_focus(command);
        else if (command[0].compare("NEW") == 0) cmd_new(command);
        else if (command[0].compare("DELETE") == 0) cmd_delete(command);
        else if (command[0].compare("SETTOGGLEKEY") == 0) cmd_setToggleKey(command);
        else std::cout << "Error: Invalid input." << std::endl;
    } while (input.compare("exit") != 0);
}

void test(int i) {
    while(true) {
        printf("Hello %d\n", i);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

int main()
{
    byte bytes[] = { 0x8b, 0xff, 0x55, 0x8b, 0xec };   // original bytes of PostMessageW
    PostMessageW_Gateway = (PostMessageWT)createFunctionGateway(PostMessageW, bytes, 5);

    std::thread t1(inputThread);

    cmd_listWindows();

    for (;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}