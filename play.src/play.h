#ifndef PLAY_H
#define PLAY_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "gameio.h"
#include "constants.h"

#include "playerror.h"


class Game {
public:
    struct ListItem {
        ListItem(std::uint32_t value, std::uint32_t chance)
        : value(value), chance(chance)
        { }

        std::uint32_t value;
        std::uint32_t chance;
    };
    struct List {
        void add(std::uint32_t value, std::uint32_t chance);
        void remove(std::uint32_t value);
        bool contains(std::uint32_t value) const;
        std::uint32_t random() const;

        std::vector<ListItem> items;
    };

    struct RuntimeData {
        RuntimeData()
        : list(nullptr)
        { }
        RuntimeData(List *list)
        : list(list)
        { }

        std::shared_ptr<List> list;
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
        std::uint32_t itemIdent;
    };

    Game(GameIO &io)
    : isRunning(false), locationName(0), io(io), data(nullptr), nextDataItem(0)
    { }
    ~Game() {
        delete[] data;
    }

    void loadDataFromFile(const std::string &filename);

    int getType(std::uint32_t address) const;
    bool isType(std::uint32_t address, uint8_t type) const;
    std::uint8_t readByte(std::uint32_t pos) const;
    std::uint16_t readShort(std::uint32_t pos) const;
    std::uint32_t readWord(std::uint32_t pos) const;
    const char *getString(std::uint32_t address) const;

    std::uint32_t getProperty(std::uint32_t address, int propId) const;
    std::uint32_t hasFlag(std::uint32_t address, std::uint32_t flags) const;
    void sayAddress(std::uint32_t address);
    bool addItems(int qty, std::uint32_t itemIdent);
    bool removeItems(int qty, std::uint32_t itemIdent);
    bool itemQty(std::uint32_t itemIdent);

    void startGame();
    void doOption(int optionNumber);
    void useItem(int itemNumber);
    void newNode(std::uint32_t address);
    void doNode(std::uint32_t address);

    bool actionAllowed() const;
    void sayError(const std::string &errorMessage);

    uint32_t fetch(uint32_t key) const;
    void push(uint32_t value);
    uint32_t pop();

    void addData(std::uint32_t ident, List *list);
    std::shared_ptr<List> getDataAsList(std::uint32_t ident);
    void freeData(std::uint32_t ident);

    bool isRunning;
    std::vector<Option> options;
    std::vector<CarriedItem> inventory;
    std::uint32_t locationName;
private:
    GameIO &io;

    std::map<std::uint32_t, std::uint32_t> storage;
    std::uint32_t location;
    bool inLocation;
    bool newLocation;
    uint8_t *data;
    size_t dataSize;
    std::vector<uint32_t> stack;
    uint32_t nextDataItem;
    std::map<std::uint32_t, RuntimeData> runtimeData;
};
#endif
