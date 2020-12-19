#include "Input.h"


Input::Input(Window window, std::vector<int> vkCodes, int delay1, int delay2, int toggleKey)
    :window(window)
{
    this->vkCodes = vkCodes;
    this->delay1 = delay1;
    this->delay2 = delay2;
    this->toggleKey = toggleKey;
    this->toggleKeyHold = false;
    this->isDestructing = false;
    this->isRunning = false;
    writeKeyThread = std::thread(&Input::thread_writeKey, this);
    readKeyThread = std::thread(&Input::thread_readKey, this);
}

Input::~Input() {
    stop();
    isDestructing = true;
    writeKeyThread.join();
    readKeyThread.join();
}

void Input::start() {
    if (!isRunning) {
        isRunning = true;
    }
}

void Input::stop() {
    if (isRunning) {
        isRunning = false;
    }
}

void Input::thread_writeKey() {
    while (!isDestructing) {
        if (isRunning) {
            for (size_t i = 0; i < vkCodes.size() && isRunning; i++) {

                // send key
                if(help::PostMessageW_Gateway != nullptr) {
                    int vkCode = vkCodes[i];
                    if (vkCode >= 0 && help::PostMessageW_Gateway(window.getHandle(), WM_KEYDOWN, vkCode, 1) == NULL) {
                        stop();
                        std::cout << "Error: Failed to send input \"" << std::string(vkCodes.begin(), vkCodes.end()) << "\" to window \"" << window.getName() << "\". Input stopped." << std::endl;
                    }
                    else if (i < vkCodes.size() - 1) {

                        // wait until delay2 passed
                        std::chrono::steady_clock::time_point delay2_start = std::chrono::steady_clock::now();
                        while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - delay2_start).count() < delay2) {
                            if (!isRunning) break;
                            std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        }
                    }
                }
                else {
                    std::cout << "Error: Failed to send input \"" << std::string(vkCodes.begin(), vkCodes.end()) << "\" to window \"" << window.getName() << "\". PostMessageW_Gateway not initialized." << std::endl;
                }
            }

            // wait until delay1 passed
            std::chrono::steady_clock::time_point delay2_start = std::chrono::steady_clock::now();
            while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - delay2_start).count() < delay1) {
                if (!isRunning) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void Input::thread_readKey() {
    while (!isDestructing) {
        if (toggleKey >= 0) {
            if (GetAsyncKeyState(toggleKey)) {
                toggleKeyHold = true;
            }
            else if (toggleKeyHold) {
                toggleKeyHold = false;
                // key released
                //printf("Key released!\n");
                if (isRunning) stop();
                else start();
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

std::vector<int> Input::getVirtualKeyCodes() {
    return vkCodes;
}

std::string Input::getKeys() {
    std::string result = "";
    for (size_t i = 0; i < vkCodes.size(); i++) {
        int vkCode = vkCodes[i];
        if (vkCode >= 0) {
            result += vkCodes[i];
        }
        else {
            result += "<NULL>";
        }
    }
    return result;
}

int Input::getDelay1() {
    return delay1;
}

int Input::getDelay2() {
    return delay2;
}

void Input::setToggleKey(int vkCode) {
    toggleKey = vkCode;
}

int Input::getToggleKey() {
    return toggleKey;
}