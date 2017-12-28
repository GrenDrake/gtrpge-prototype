#ifndef PLAY_H
#define PLAY_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "gameio.h"
#include "constants.h"

#include "playerror.h"

struct CharacterDef {
    std::uint32_t address;
    std::uint32_t article, name;
    std::uint32_t sex, species;
    std::uint32_t faction;
};
struct Character {
    const CharacterDef *def;
    std::uint32_t sex, species;
};

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
        enum Type {
            NoneType, ListType, CharacterType
        };
        RuntimeData()
        : list(nullptr), character(nullptr)
        { }
        RuntimeData(List *list)
        : list(list), character(nullptr)
        { }
        RuntimeData(Character *character)
        : list(nullptr), character(character)
        { }

        Type getType() const {
            if (list) return ListType;
            if (character) return CharacterType;
            return NoneType;
        }

        std::shared_ptr<List> list;
        std::shared_ptr<Character> character;
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

    Game()
    : isRunning(false), locationName(0), data(nullptr), nextDataItem(0)
    { }
    ~Game() {
        delete[] data;
    }

    void loadDataFromFile(const std::string &filename);

    void clearOutput();
    std::string getOutput() const;

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

    const CharacterDef* getCharacterDef(std::uint32_t address);
    std::uint32_t makeCharacter(std::uint32_t defAddress);

    void startGame();
    void doOption(int optionNumber);
    void useItem(int itemNumber);
    void newNode(std::uint32_t address);
    void doNode(std::uint32_t address);

    bool actionAllowed() const;
    void say(const std::string text);
    void say(int number);
    void sayError(const std::string &errorMessage);

    uint32_t fetch(uint32_t key) const;
    void push(uint32_t value);
    uint32_t pop();

    void addData(std::uint32_t ident, List *list);
    void addData(std::uint32_t ident, Character *character);
    RuntimeData::Type dataType(std::uint32_t ident);
    std::shared_ptr<List> getDataAsList(std::uint32_t ident);
    std::shared_ptr<Character> getDataAsCharacter(std::uint32_t ident);
    void freeData(std::uint32_t ident);

    bool isRunning;
    std::vector<Option> options;
    std::vector<CarriedItem> inventory;
    std::uint32_t locationName;

private:
    std::map<std::uint32_t, std::uint32_t> storage;
    std::uint32_t location;
    bool inLocation;
    bool newLocation;
    uint8_t *data;
    size_t dataSize;
    std::vector<uint32_t> stack;
    uint32_t nextDataItem;
    std::map<std::uint32_t, RuntimeData> runtimeData;
    std::map<std::uint32_t, const CharacterDef*> characterDefs;
    std::string outputBuffer;
};
#endif
