#pragma once

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include "Window.h"
#include "helpers.h"


class Input
{
private:
    Window window;
    std::vector<int> vkCodes;
    int delay1, delay2;
    int toggleKey;
    bool toggleKeyHold;
    bool isDestructing, isRunning;
    std::thread writeKeyThread, readKeyThread;

public:
    Input(Window window, std::vector<int> vkCodes, int delay1, int delay2, int toggleKey = -1);
    ~Input();

    void start();
    void stop();

    void thread_writeKey();
    void thread_readKey();

    std::vector<int> getVirtualKeyCodes();
    int getDelay1();
    int getDelay2();
    std::string getKeys();

    void setToggleKey(int vkCode);
    int getToggleKey();
};

