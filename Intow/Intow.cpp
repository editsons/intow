#include <iostream>
#include <Windows.h>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include "Window.h"
#include "helpers.h"
#include "Input.h"


std::vector<Window> g_windowList;
std::vector<Input*> g_inputList;





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
    help::fetchWindows(g_windowList, filter);
    for (size_t i = 0; i < g_windowList.size(); i++) {
        std::cout << i << ": " << g_windowList[i].getName() << std::endl;
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
    std::cout << "ID  :  keys  :  delay1  : delay2  :  toggleKey" << std::endl;
    for (size_t i = 0; i < g_inputList.size(); i++) {
        //std::string(g_inputList[i]->getVirtualKeyCodes().begin(), g_inputList[i]->getVirtualKeyCodes().end()) <<

        std::cout << i << ": " << g_inputList[i]->getKeys() << " : " << g_inputList[i]->getDelay1() << " : " << g_inputList[i]->getDelay2() <<  std::endl;
    }
    std::cout << std::endl;
}
void cmd_focus(std::vector<std::string>& command) {
    if (command.size() > 1) {
        size_t windowId = std::stoi(command[1]);
        if (windowId >= 0 && windowId < g_windowList.size()) {
            SetForegroundWindow(g_windowList[windowId].getHandle());
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
        size_t windowId = std::stoi(command[1]);
        if (windowId >= 0 && windowId < g_windowList.size()) {
            std::vector<int> virtualKeyCodes = help::sstovkc(command[2]);
            if (virtualKeyCodes.size() > 0) {
                int delay2 = command.size() < 5 ? 0 : std::stoi(command[4]);
                g_inputList.push_back(new Input(g_windowList[windowId], virtualKeyCodes, std::stoi(command[3]), delay2, help::stovkc("ALT")));
                g_inputList.back()->start();
                std::cout << "Successfully created input." << std::endl << std::endl;
            }
            else {
                std::cout << "Error: Failed to parse keys." << std::endl << std::endl;
            }
        }
        else {
            std::cout << "Error: Invalid window id." << std::endl << std::endl;
        }
    }
    else {
        std::cout << "Usage: new <windowId> <keys> <delay1_ms> [<delay2_ms>]" << std::endl << std::endl;
    }
}
void cmd_delete(std::vector<std::string>& command) {
    if (command.size() >= 2) {
        std::regex reg("^[0-9]$");
        if (std::regex_match(command[1], reg)) {
            try {
                size_t inputId = std::stoi(command[1]); // TODO: catch errors of stoi
                if (inputId >= 0 && inputId < g_inputList.size()) {
                    delete g_inputList[inputId];
                    g_inputList.erase(g_inputList.begin() + inputId);
                    std::cout << "Successfully deleted input." << std::endl << std::endl;
                }
                else {
                    std::cout << "Error: Input with id " << inputId << " does not exist." << std::endl << std::endl;
                }
            }
            // Standard exceptions for stoi
            catch (const std::invalid_argument& e) {
                //std::cout << e.what() << "\n";
                std::cout << "Error: Invalid input id." << std::endl << std::endl;
            }
            catch (const std::out_of_range& e) {
                //std::cout << e.what() << "\n";
                std::cout << "Error: Invalid input id." << std::endl << std::endl;
            }
        }
        else {
            std::cout << "Error: Invalid input id." << std::endl << std::endl;
        }
    }
    else {
        std::cout << "Usage: delete <inputId>" << std::endl << std::endl;
    }
}
void cmd_setToggleKey(std::vector<std::string>& command) {
    if (command.size() > 2) {
        size_t inputId = std::stoi(command[1]);
        std::string key = command[2];
        if (inputId >= 0 && inputId < g_inputList.size()) {
            std::vector<int> vkCodes = help::sstovkc(key);
            if (vkCodes.size() > 0) {
                unsigned char vkCode = vkCodes.at(0);
                g_inputList[inputId]->setToggleKey(vkCode);
                std::cout << "Successfully set toggle key." << std::endl << std::endl;
            }
            else {
                std::cout << "Error: Invalid key." << std::endl << std::endl;
            }
        }
        else {
            std::cout << "Error: Invalid input id." << std::endl << std::endl;
        }
    }
    else {
        std::cout << "Usage: setToggleKey <inputId> <key>" << std::endl << std::endl;
    }
}
void cmd_stop(std::vector<std::string>& command) {
    if (command.size() > 1) {
        size_t inputId = std::stoi(command[1]);
        if (inputId >= 0 && inputId < g_inputList.size()) {
            g_inputList[inputId]->stop();
        }
        else {
            std::cout << "Error: Invalid input id." << std::endl << std::endl;
        }
    }
    else {
        std::cout << "Usage: stop <inputId>" << std::endl << std::endl;
    }
}
void cmd_start(std::vector<std::string>& command) {
    if (command.size() > 1) {
        size_t inputId = std::stoi(command[1]);
        if (inputId >= 0 && inputId < g_inputList.size()) {
            g_inputList[inputId]->start();
        }
        else {
            std::cout << "Error: Invalid input id." << std::endl << std::endl;
        }
    }
    else {
        std::cout << "Usage: start <inputId>" << std::endl << std::endl;
    }
}


void inputThread() {
    std::string input;
    do {
        std::getline(std::cin, input);
        std::vector<std::string> command = help::splitSentenceIntoWords(input);
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
        else if (command[0].compare("STOP") == 0) cmd_stop(command);
        else if (command[0].compare("START") == 0) cmd_start(command);
        else std::cout << "Error: Invalid input." << std::endl << std::endl;
    } while (input.compare("exit") != 0);
}


int main()
{
    help::PostMessageW_Gateway_init();
    cmd_listWindows();
    std::thread t1(inputThread);
    t1.join();
    //std::this_thread::sleep_for(std::chrono::milliseconds(9999999999));
    return 0;
}