#ifndef PLAY_H
#define PLAY_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "gameio.h"
#include "constants.h"

class PlayError : public std::runtime_error {
public:
        PlayError(const std::string &msg)
        : std::runtime_error(msg)
        { }
};



class Option {
public:
    Option()
    : dest(0)
    { }
    Option(std::uint32_t name, std::uint32_t destination, std::uint32_t extra = 0)
    : name(name), dest(destination), extra(extra)
    { }

    std::uint32_t name;
    std::uint32_t dest;
    std::uint32_t extra;
};

class CarriedItem {
public:
    CarriedItem()
    : qty(0), itemIdent(0)
    { }
    CarriedItem(int qty, int itemIdent)
    : qty(qty), itemIdent(itemIdent)
    { }

    int qty;
    int itemIdent;
};

class Game {
public:
    Game(GameIO &io)
    : isRunning(false), locationName(0), io(io), data(nullptr)
    { }
    ~Game() {
        delete[] data;
    }

    void loadDataFromFile(const std::string &filename);

    std::uint8_t readByte(std::uint32_t pos) const;
    std::uint16_t readShort(std::uint32_t pos) const;
    std::uint32_t readWord(std::uint32_t pos) const;
    const char *getString(std::uint32_t address) const;

    void startGame();
    void doOption(int optionNumber);
    void newNode(std::uint32_t address);
    void doNode(std::uint32_t address);

    bool actionAllowed() const;

    void push(uint32_t value);
    uint32_t pop();

    bool isRunning;
    std::vector<Option> options;
    std::vector<CarriedItem> inventory;
    std::uint32_t locationName;
private:
    int nextWord(std::uint32_t &ip) {
        std::uint32_t v = readWord(ip);
        ip += 4;
        return v;
    }

    GameIO &io;

    std::map<std::uint32_t, std::uint32_t> storage;
    std::uint32_t location;
    bool inLocation;
    bool newLocation;
    uint8_t *data;
    std::vector<uint32_t> stack;
};
#endif
