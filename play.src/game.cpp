#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "play.h"


void Game::List::add(std::uint32_t value, std::uint32_t chance) {
    for (ListItem &item : items) {
        if (item.value == value) {
            item.chance = chance;
            return;
        }
    }
    items.push_back(ListItem(value, chance));
}
void Game::List::remove(std::uint32_t value) {
    for (auto item = items.begin(); item != items.end(); ++item) {
        if (item->value == value) {
            items.erase(item);
            return;
        }
    }
}
bool Game::List::contains(std::uint32_t value) const {
    for (const ListItem &item : items) {
        if (item.value == value) {
            return true;
        }
    }
    return false;
}
std::uint32_t Game::List::random() const {
    std::vector<std::uint32_t> deck;
    for (const ListItem &item : items) {
        for (unsigned i = 0; i < item.chance; ++i) {
            deck.push_back(item.value);
        }
    }
    unsigned i = rand() % deck.size();
    return deck[i];
}



void Game::loadDataFromFile(const std::string &filename) {
    std::ifstream inf(filename, std::ios::binary | std::ios::in | std::ios::ate);
    if (!inf) {
        throw PlayError("Could not read game data from "+filename+".");
    }
    std::streamsize size = inf.tellg();
    inf.seekg(0);

    data = new std::uint8_t[size];
    if (!inf.read((char*)data, size)) {
        throw PlayError("Could not read game data.");
    }

    dataSize = size;
    nextDataItem = size + 32;
}

void Game::clearOutput() {
    outputBuffer = "";
}

std::string Game::getOutput() const {
    return outputBuffer;
}

int Game::getType(std::uint32_t address) const {
    return readByte(address);
}

bool Game::isType(std::uint32_t address, uint8_t type) const {
    return readByte(address) == type;
}

std::uint8_t Game::readByte(std::uint32_t pos) const {
    if (!data) return 0;
    if (pos >= dataSize) throw PlayError("Tried to read past end of file.");
    return (uint8_t)data[pos];
}

std::uint16_t Game::readShort(std::uint32_t pos) const {
    if (!data) return 0;
    if (pos >= dataSize) throw PlayError("Tried to read past end of file.");
    std::uint16_t v = 0;
    v |= (unsigned char)data[pos];
    v |= (unsigned char)data[pos+1] << 8;
    return v;
}

std::uint32_t Game::readWord(std::uint32_t pos) const {
    if (!data) return 0;
    if (pos >= dataSize) throw PlayError("Tried to read past end of file.");
    std::uint32_t result = 0, value;
    value = data[pos];  result |= value;
    value = data[pos+1];  result |= value << 8;
    value = data[pos+2];  result |= value << 16;
    value = data[pos+3];  result |= value << 24;
    return result;
}

const char *Game::getString(std::uint32_t address) const {
    if (!isType(address, idString)) {
        std::stringstream ss;
        ss << "Tried to read non-string at address 0x" << std::hex << std::uppercase << address;
        ss << " as a string.";
        throw PlayError(ss.str());
    }
    return reinterpret_cast<const char*>(&data[address+1]);
}

std::uint32_t Game::getProperty(std::uint32_t address, int propId) const {
    return readWord(address + propId);
}

std::uint32_t Game::hasFlag(std::uint32_t address, std::uint32_t flags) const {
    std::uint32_t curFlags = 0;
    flags = 1 << flags;
    switch(getType(address)) {
        case idItem:
            curFlags = getProperty(address, itmFlags);
            return (flags & curFlags) == flags;
        default: {
            std::stringstream ss;
            ss << "Tried to test flag of unflagged object type ";
            ss << getType(address) << " at 0x" << std::hex << address << ".";
            throw PlayError(ss.str());
        }
    }
}

void Game::sayAddress(std::uint32_t address) {
    if (address > dataSize) {
        std::stringstream ss;
        switch(dataType(address)) {
            case RuntimeData::Type::NoneType:
                ss << "[unused address#";
                ss << std::hex << std::setw(8) << std::setfill('0');
                ss << address << "]";
                say(ss.str());
                break;
            case RuntimeData::Type::CharacterType: {
                std::shared_ptr<Character> c = getDataAsCharacter(address);
                ss << getString(c->def->article);
                ss << getString(c->def->name);
                say(ss.str());
                break;
            }
            default:
                ss << std::hex << std::setw(8) << std::setfill('0');
                ss << "[high memory object#"<<address<<"]";
                say(ss.str());
        }
        return;
    }

    int type = getType(address);
    int work;
    switch(type) {
        case idString:
            say(getString(address));
            break;
        case idItem:
            work = address + itmArticle;
            say(getString(readWord(work)));
            work = address + itmSingular;
            say(getString(readWord(work)));
            break;
        case idSex:
            work = address + sexName;
            say(getString(readWord(work)));
            break;
        case idSpecies:
            work = address + spcName;
            say(getString(readWord(work)));
            break;
        default: {
            std::stringstream ss;
            ss << "[object#";
            ss << std::hex << std::uppercase << address;
            ss << '/' << (int) type << ']';
            say(ss.str());
        }
    }
}

bool Game::addItems(int qty, std::uint32_t itemIdent) {
    for (unsigned i = 0; i < inventory.size(); ++i) {
        if (inventory[i].itemIdent == itemIdent) {
            inventory[i].qty += qty;
            return true;
        }
    }
    inventory.push_back(CarriedItem(qty, itemIdent));
    std::sort(inventory.begin(), inventory.end(), [this](const Game::CarriedItem &a, const Game::CarriedItem &b) {
        const char *aName = getString(getProperty(a.itemIdent, itmSingular));
        const char *bName = getString(getProperty(b.itemIdent, itmSingular));
        return strcmp(aName, bName) < 0;
    });
    return true;
}

bool Game::removeItems(int qty, std::uint32_t itemIdent) {
    for (auto ci = inventory.begin(); ci != inventory.end(); ++ci) {
        if (ci->itemIdent == itemIdent) {
            if (ci->qty >= qty) {
                ci->qty -= qty;
                if (ci->qty <= 0) {
                    inventory.erase(ci);
                }
                return true;
            }
        }
    }
    return false;
}

bool Game::itemQty(std::uint32_t itemIdent) {
    for (CarriedItem &ci : inventory) {
        if (ci.itemIdent == itemIdent) {
            return ci.qty;
        }
    }
    return 0;
}

/*
const int chrSkillList      = 25;
const int chrGearList       = 29;
const int chrSize           = 33;
const int csIdent           = 0;
const int csValue           = 4;
const int csSize            = 8;
const int cgSlot            = 0;
const int cgItem            = 4;
const int cgSize            = 8;
*/

const CharacterDef* Game::getCharacterDef(std::uint32_t address) {
    if (!isType(address, idCharacter)) {
        std::stringstream ss;
        ss << "Tried to create character from non-character data at 0x";
        ss << std::hex << std::setw(8) << std::setfill('0') << address;
        throw PlayError(ss.str());
    }

    auto i = characterDefs.find(address);
    if (i == characterDefs.end()) {
        CharacterDef *cdef = new CharacterDef;
        cdef->address = address;
        cdef->article = getProperty(address, chrArticle);
        cdef->name = getProperty(address, chrName);
        cdef->sex = getProperty(address, chrSex);
        cdef->species = getProperty(address, chrSpecies);
        cdef->faction = getProperty(address, chrFaction);
        characterDefs.insert(std::make_pair(address, cdef));
        return cdef;
    } else {
        return i->second;
    }
}

std::uint32_t Game::makeCharacter(std::uint32_t defAddress) {
    const CharacterDef *def = getCharacterDef(defAddress);
    std::uint32_t ident = nextDataItem++;
    Character *c = new Character;
    c->def = def;
    c->sex = def->sex;
    c->species = def->species;
    addData(ident, c);
    return ident;
}

void Game::startGame() {
    location = 0;
    isRunning = true;
    say(getString(readWord(headerTitle)));
    say(" (");
    say(getString(readWord(headerVersion)));
    say(")\n");
    say(getString(readWord(headerByline)));
    say("\n\n");
    srand(time(nullptr));
    newNode(readWord(headerStartNode));
}

void Game::newNode(std::uint32_t address) {
    inLocation = false;
    newLocation = false;
    options.clear();
    try {
        doNode(address);
    } catch (PlayError &e) {
        sayError(e.what());
    }
    stack.clear();
    say("\n");
}

void Game::doOption(int optionNumber) {
    if (optionNumber < 0 || optionNumber >= (signed)options.size()) {
        return;
    }

    std::uint32_t dest = options[optionNumber].dest;
    if (dest == 0) {
        return;
    }

    clearOutput();
    say("\n> ");
    uint32_t nameAddr = options[optionNumber].name;
    if (nameAddr == 1) {
        say("Continue");
    } else {
        say(getString(nameAddr));
    }
    say("\n\n");

    storage[xtraValue] = options[optionNumber].extra;
    newNode(dest);
}

void Game::useItem(int itemNumber) {
    if (itemNumber < 0 || itemNumber >= (signed)inventory.size()) {
        return;
    }

    uint32_t item = inventory[itemNumber].itemIdent;
    if (!item) return;

    uint32_t onUse = getProperty(item, itmOnUse);
    if (!onUse) return;

    uint32_t article = getProperty(item, itmArticle);
    uint32_t name = getProperty(item, itmSingular);

    clearOutput();
    say("\n> Using ");
    say(getString(article));
    say(getString(name));
    say("\n\n");

    newNode(onUse);
}


bool Game::actionAllowed() const {
    return inLocation && isRunning;
}

void Game::say(const std::string text) {
    outputBuffer += text;
}

void Game::say(int number) {
    say(std::to_string(number));
}

void Game::sayError(const std::string &errorMessage) {
    say("\n");
    say(errorMessage);
    say("\nYour game may be in an invalid state; restoring or restarting is recommended.\n");
}

uint32_t Game::fetch(uint32_t key) const {
    if (storage.count(key) == 0) {
        return 0;
    }
    auto i = storage.find(key);
    return i->second;
}

void Game::push(uint32_t value) {
    stack.push_back(value);
}

uint32_t Game::pop() {
    if (stack.empty()) return 0;
    uint32_t value = stack.back();
    stack.pop_back();
    return value;
}

void Game::addData(std::uint32_t ident, List *list) {
    RuntimeData data(list);
    runtimeData.insert(std::make_pair(ident, data));
}

void Game::addData(std::uint32_t ident, Character *character) {
    RuntimeData data(character);
    runtimeData.insert(std::make_pair(ident, data));
}

Game::RuntimeData::Type Game::dataType(std::uint32_t ident) {
    auto i = runtimeData.find(ident);
    if (i == runtimeData.end()) {
        return RuntimeData::Type::NoneType;
    }
    return i->second.getType();
}

std::shared_ptr<Game::List> Game::getDataAsList(std::uint32_t ident) {
    auto i = runtimeData.find(ident);
    if (i != runtimeData.end()) {
        return i->second.list;
    }
    return nullptr;
}

std::shared_ptr<Character> Game::getDataAsCharacter(std::uint32_t ident) {
    auto i = runtimeData.find(ident);
    if (i != runtimeData.end()) {
        return i->second.character;
    }
    return nullptr;
}

void Game::freeData(std::uint32_t ident) {
    auto i = runtimeData.find(ident);
    if (i != runtimeData.end()) {
        runtimeData.erase(i);
    }
}