#include <string>

#include "gameio.h"
extern "C" {
#include "glk.h"
}

static winid_t mainWindow = nullptr;
static winid_t optionsWindow = nullptr;
static winid_t statusWindow = nullptr;
static winid_t sidebarWindow = nullptr;
static winid_t currentWindow = nullptr;

GameIO::GameIO() {
    mainWindow = glk_window_open(0, 0, 0, wintype_TextBuffer, 1);
    if (!mainWindow) {
        return;
    }
    statusWindow = glk_window_open(mainWindow,
        winmethod_Above | winmethod_Fixed,
        1, wintype_TextGrid, 0);

    optionsWindow = glk_window_open(mainWindow,
        winmethod_Below | winmethod_Fixed,
        5, wintype_TextBuffer, 0);
    sidebarWindow = glk_window_open(mainWindow,
        winmethod_Right | winmethod_Proportional,
        33, wintype_TextBuffer, 0);
    glk_set_window(mainWindow);
    currentWindow = mainWindow;
}
GameIO::~GameIO() {
}

void GameIO::say(const std::string &text) {
    glk_put_string(const_cast<char*>(text.c_str()));
}

void GameIO::say(int value) {
    say(std::to_string(value));
}

int GameIO::getKey() {
    event_t event;

    glk_request_char_event(optionsWindow);
    while (true) {
        glk_select(&event);

        switch (event.type) {
            case evtype_CharInput:
                return event.val1;
        }
    }
}

std::string GameIO::getLineRaw(const std::string &initial) {
    event_t event;

    char inBuffer[128] = {0};
    int initLen = initial.size();
    if (initLen) {
        strncpy(inBuffer, initial.c_str(), 127);
    }

    glk_request_line_event(optionsWindow, inBuffer, 127, initLen);
    while (true) {
        glk_select(&event);

        switch (event.type) {
            case evtype_LineInput:
                inBuffer[event.val1] = 0;
                return inBuffer;
        }
    }
}

std::string GameIO::getLine(const std::string &prompt, const std::string &initial) {
    setWindow(GameIO::Options);
    clear();
    say(prompt);
    std::string s = getLineRaw(initial);
    setWindow(GameIO::Main);
    return s;
}


void GameIO::style(Style style) {
    switch(style) {
        case Normal:
            glk_set_style(style_Normal);
            break;
        case Emphasis:
            glk_set_style(style_Emphasized);
            break;
        case Bold:
            glk_set_style(style_Alert);
            break;
    }
}

void GameIO::setWindow(Window window) {
    switch(window) {
        case Status:
            currentWindow = (statusWindow);
            break;
        case Main:
            currentWindow = (mainWindow);
            break;
        case Options:
            currentWindow = (optionsWindow);
            break;
        case Sidebar:
            currentWindow = (sidebarWindow);
            break;
    }
    glk_set_window(currentWindow);
}

void GameIO::clear() {
    glk_window_clear(currentWindow);
}
