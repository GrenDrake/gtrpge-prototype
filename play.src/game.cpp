#include <iomanip>
#include <fstream>
#include <sstream>

#include "play.h"

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
}

int Game::getType(std::uint32_t address) const {
    return readByte(address);
}

bool Game::isType(std::uint32_t address, uint8_t type) const {
    return readByte(address) == type;
}

std::uint8_t Game::readByte(std::uint32_t pos) const {
    if (!data) return 0;
    return (uint8_t)data[pos];
}

std::uint16_t Game::readShort(std::uint32_t pos) const {
    if (!data) return 0;
    std::uint16_t v = 0;
    v |= (unsigned char)data[pos];
    v |= (unsigned char)data[pos+1] << 8;
    return v;
}

std::uint32_t Game::readWord(std::uint32_t pos) const {
    if (!data) return 0;
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
    int type = getType(address);
    int work;
    switch(type) {
        case idString:
            io.say(getString(address));
            break;
        case idItem:
            work = address + itmArticle;
            io.say(getString(readWord(work)));
            work = address + itmSingular;
            io.say(getString(readWord(work)));
            break;
        default: {
            std::stringstream ss;
            ss << "[object#";
            ss << std::hex << std::uppercase << address;
            ss << '/' << (int) type << ']';
            io.say(ss.str());
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

void Game::startGame() {
    location = 0;
    isRunning = true;
    io.say(getString(readWord(headerTitle)));
    io.say(" (");
    io.say(getString(readWord(headerVersion)));
    io.say(")\n");
    io.say(getString(readWord(headerByline)));
    io.say("\n\n");
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
    io.say("\n");
}

int Game::nextOperand(std::uint32_t &ip) {
    std::uint32_t v = readWord(ip);
    ip += 4;

    if (v == stackOperand) {
        return pop();
    } else {
        return v;
    }
}

void Game::doOption(int optionNumber) {
    if (optionNumber < 0 || optionNumber >= (signed)options.size()) {
        return;
    }

    std::uint32_t dest = options[optionNumber].dest;
    if (dest == 0) {
        return;
    }

    io.style(GameIO::Emphasis);
    io.say("\n> ");
    uint32_t nameAddr = options[optionNumber].name;
    if (nameAddr == 1) {
        io.say("Continue");
    } else {
        io.say(getString(nameAddr));
    }
    io.say("\n\n");
    io.style(GameIO::Normal);

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

    io.style(GameIO::Emphasis);
    io.say("\n> Using ");
    io.say(getString(article));
    io.say(getString(name));
    io.say("\n\n");
    io.style(GameIO::Normal);

    newNode(onUse);
}


bool Game::actionAllowed() const {
    return inLocation && isRunning;
}

void Game::sayError(const std::string &errorMessage) {
    io.style(GameIO::Bold);
    io.say("INTERNAL ERROR: ");
    io.say(errorMessage);
    io.say("\nYour game may be in an invalid state; restoring or restarting is recommended.");
    io.style(GameIO::Normal);
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
