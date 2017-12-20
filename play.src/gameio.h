#ifndef GAMEIO_H
#define GAMEIO_H

#include <string>

class GameIO {
public:
    enum Window {
        Status, Main, Options, Sidebar
    };
    enum Style {
        Normal, Emphasis, Bold
    };

    void setup();
    void say(const std::string &text);
    void say(int value);
    int getKey();
    void style(Style style = Normal);
    void setWindow(Window window);
    void clear();
private:
};

#endif