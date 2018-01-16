#ifndef PLAY_H
#define PLAY_H

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "constants.h"

#include "playerror.h"

class Game {
public:
    struct Character {
        std::uint32_t def;
        std::uint32_t sex, species;
        std::array<std::int8_t, sklCount> skillAdj;
        std::array<std::uint8_t, sklCount> skillCur;
        std::map<std::uint32_t, std::uint32_t> gear;
    };

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
    : gameStarted(false), locationName(0), isRunning(false), data(nullptr),
      nextDataItem(0), gameTime(0)
    { }
    ~Game() {
        delete[] data;
    }

    // ////////////////////////////////////////////////////////////////////////
    // Game Engine Startup                                                   //
    void loadDataFromFile(const std::string &filename);
    void setDataAs(uint8_t *data, size_t size);

    // ////////////////////////////////////////////////////////////////////////
    // Fetching game state                                                   //
    std::string getTimeString(bool exact = false);
    std::string getOutput() const;

    std::uint32_t getProperty(std::uint32_t address, int propId) const;
    std::uint32_t hasFlag(std::uint32_t address, std::uint32_t flags) const;
    std::string getNameOf(std::uint32_t address);

    Character* getCharacter(std::uint32_t address);
    bool testSkillFlags(int skillNo, uint32_t flags);
    int getSkillMax(std::uint32_t cRef, int skillNo);
    int getSkillCur(std::uint32_t cRef, int skillNo);
    std::vector<std::uint32_t> getActions(std::uint32_t cRef);

    std::uint8_t readByte(std::uint32_t pos) const;
    std::uint16_t readShort(std::uint32_t pos) const;
    std::uint32_t readWord(std::uint32_t pos) const;

    // ////////////////////////////////////////////////////////////////////////
    // Player action commands                                                //
    bool actionAllowed() const;
    void doOption(int optionNumber);
    void useItem(int itemNumber);
    void equipItem(std::uint32_t whoIdent, int itemNumber);
    void unequipItem(std::uint32_t whoIdent, std::uint32_t slotIdent);
    void doAction(std::uint32_t cRef, std::uint32_t action);


    // ////////////////////////////////////////////////////////////////////////
    // Public game state data                                                //
    bool gameStarted;
    std::vector<Option> options;
    std::vector<CarriedItem> inventory;
    std::uint32_t locationName;
    std::vector<std::uint32_t> party;
private:
    // ////////////////////////////////////////////////////////////////////////
    // Game Engine Startup                                                   //
    void doGameSetup();
    static int roll(int dice, int sides);

    // ////////////////////////////////////////////////////////////////////////
    // Raw Data Management                                                   //
    int getType(std::uint32_t address) const;
    bool isType(std::uint32_t address, uint8_t type) const;
    const char *getString(std::uint32_t address) const;

    // ////////////////////////////////////////////////////////////////////////
    // Character Management                                                  //
    void resetCharacter(std::uint32_t cRef);
    void doDamage(std::uint32_t cRef, int amount, int to, int type);
    int doSkillCheck(std::uint32_t cRef, int skill, int modifiers, int target);
    void adjSkillMax(std::uint32_t cRef, int skillNo, int adjustment);
    void adjSkillCur(std::uint32_t cRef, int skillNo, int adjustment);

    // ////////////////////////////////////////////////////////////////////////
    // node execution                                                        //
    void newNode(std::uint32_t address);
    void doNode(std::uint32_t address);

    // ////////////////////////////////////////////////////////////////////////
    // Output manipulation                                                   //
    void clearOutput();
    void say(const std::string &text);
    void say(int number);
    void sayError(const std::string &errorMessage);

    // ////////////////////////////////////////////////////////////////////////
    // inventory management                                                  //
    bool addItems(int qty, std::uint32_t itemIdent);
    bool removeItems(int qty, std::uint32_t itemIdent);
    bool itemQty(std::uint32_t itemIdent);

    // ////////////////////////////////////////////////////////////////////////
    // dynamic list management                                               //
    void addData(std::uint32_t ident, List *list);
    RuntimeData::Type dataType(std::uint32_t ident);
    std::shared_ptr<List> getDataAsList(std::uint32_t ident);
    void freeData(std::uint32_t ident);

    // ////////////////////////////////////////////////////////////////////////
    // stack and stored data management                                      //
    uint32_t fetch(uint32_t key) const;
    void push(uint32_t value);
    uint32_t pop();


    // ////////////////////////////////////////////////////////////////////////
    // Private data storage                                                  //
    bool isRunning;
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
    unsigned gameTime;
};

std::string toTitleCase(std::string text);
std::string toUpperFirst(std::string text);
std::string trim(std::string text);
std::vector<std::string> explodeString(const std::string &text, int onChar = '\n');
std::vector<std::string> wrapString(const std::string &text, unsigned width);

const int hoursPerDay = 24;
const int minutesPerDay = 1440;
const int minutesPerHour = 60;

#endif
