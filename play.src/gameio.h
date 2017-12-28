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

    GameIO();
    ~GameIO();

    void say(const std::string &text);
    void say(int value);
    int getKey();
    std::string getLineRaw(const std::string &initial);
    std::string getLine(const std::string &prompt, const std::string &initial = "");
    void style(Style style = Normal);
    void setWindow(Window window);
    void clear();

    void startTranscript();
    void stopTranscript();
    bool hasTranscript();
private:
};

#endif