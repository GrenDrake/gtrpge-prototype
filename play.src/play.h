#ifndef PLAY_H
#define PLAY_H

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "gameio.h"
#include "constants.h"

#include "playerror.h"

struct Character {
    std::uint32_t def;
    std::uint32_t sex, species;
    std::array<std::int8_t, sklCount> skillAdj;
    std::array<std::uint8_t, sklCount> skillCur;
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
            NoneType, ListType
        };
        RuntimeData()
        : list(nullptr)
        { }
        RuntimeData(List *list)
        : list(list)
        { }

        Type getType() const {
            if (list) return ListType;
            return NoneType;
        }

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

    Game()
    : isRunning(false), locationName(0), data(nullptr), nextDataItem(0)
    { }
    ~Game() {
        delete[] data;
    }

    static int roll(int dice, int sides);

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
    std::string getNameOf(std::uint32_t address);
    void sayAddress(std::uint32_t address);
    bool addItems(int qty, std::uint32_t itemIdent);
    bool removeItems(int qty, std::uint32_t itemIdent);
    bool itemQty(std::uint32_t itemIdent);

    Character* getCharacter(std::uint32_t address);
    void resetCharacter(std::uint32_t cRef);
    int doSkillCheck(std::uint32_t cRef, int skill, int modifiers, int target);
    bool testSkillFlags(int skillNo, uint32_t flags);
    int getSkillMax(std::uint32_t cRef, int skillNo);
    void adjSkillMax(std::uint32_t cRef, int skillNo, int adjustment);
    int getSkillCur(std::uint32_t cRef, int skillNo);
    void adjSkillCur(std::uint32_t cRef, int skillNo, int adjustment);

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
    RuntimeData::Type dataType(std::uint32_t ident);
    std::shared_ptr<List> getDataAsList(std::uint32_t ident);
    void freeData(std::uint32_t ident);

    bool isRunning;
    std::vector<Option> options;
    std::vector<CarriedItem> inventory;
    std::uint32_t locationName;
    std::vector<std::uint32_t> party;
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
    std::map<std::uint32_t, Character*> characters;
    std::string outputBuffer;
};
#endif
