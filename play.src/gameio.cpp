#include <string>

#include "gameio.h"
extern "C" {
#include "glk.h"
}

static winid_t mainWindow = nullptr;
static winid_t optionsWindow = nullptr;
static winid_t statusWindow = nullptr;
static winid_t currentWindow = nullptr;

void GameIO::setup() {
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
    glk_set_window(mainWindow);
    currentWindow = mainWindow;
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
    }
    glk_set_window(currentWindow);
}

void GameIO::clear() {
    glk_window_clear(currentWindow);
}
