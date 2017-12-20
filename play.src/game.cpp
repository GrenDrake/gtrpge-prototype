#include <iostream>
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

    std::stringstream ss;
    ss << "Read " << size << " bytes of game data from " << filename << ".\n\n";
    io.say(ss.str());
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
    if (readByte(address) != idString) {
        std::stringstream ss;
        ss << "Tried to read non-string at address 0x" << std::hex << address;
        ss << " as a string.";
        throw PlayError(ss.str());
    }
    return reinterpret_cast<const char*>(&data[address+1]);
}

void Game::startGame() {
    location = 0;
    isRunning = true;
    newNode(readWord(headerStartNode));
}

void Game::newNode(std::uint32_t address) {
    inLocation = false;
    newLocation = false;
    options.clear();
    doNode(address);
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
void Game::doNode(std::uint32_t address) {
    std::uint32_t ip = address;
    if (readByte(ip++) != idNode) {
        std::stringstream ss;
        ss << "Tried to run non-node at " << std::hex << (int)readByte(ip-1) << ".";
        throw PlayError(ss.str());
    }

    std::uint32_t a1, a2, a3;
    while (true) {
        std::uint8_t cmdCode = readByte(ip++);
        switch(cmdCode) {
            case opEnd:
                return;
            case opDoNode:
                doNode(nextOperand(ip));
                break;
            case opSetLocation:
                a1 = nextOperand(ip);
                inLocation = true;
                if (locationName != a1) {
                    newLocation = true;
                    locationName = a1;
                    location = address;
                }
                break;
            case opPush:
                a1 = nextOperand(ip);
                push(a1);
                break;
            case opPop:
                pop();
                break;

            case opAddOption:
                a1 = nextOperand(ip);
                a2 = nextOperand(ip);
                options.push_back(Option(a1, a2));
                break;
            case opAddOptionXtra:
                a1 = nextOperand(ip);
                a2 = nextOperand(ip);
                a3 = nextOperand(ip);
                options.push_back(Option(a1, a2, a3));
                break;
            case opAddContinue:
                a1 = nextOperand(ip);
                options.push_back(Option(1, a1));
                break;
            case opAddReturn:
                options.push_back(Option(1, location));
                break;

            case opSay:
                io.say(getString(nextOperand(ip)));
                break;
            case opSayNumber:
                io.say(nextOperand(ip));
                break;

            case opJump:
                ip = nextOperand(ip);
                break;
            case opJumpTrue:
                a1 = pop();
                a2 = nextOperand(ip);
                if (a1) {
                    ip = a2;
                }
                break;
            case opJumpFalse:
                a1 = pop();
                a2 = nextOperand(ip);
                if (!a1) {
                    ip = a2;
                }
                break;

/* - needs implemented
const int opJumpEq          = 0x33;
const int opJumpNeq         = 0x34;
const int opJumpLt          = 0x35;
const int opJumpLte         = 0x36;
const int opJumpGt          = 0x37;
const int opJumpGte         = 0x38;
const int opForget          = 0x43;
*/

            case opStore:
                a1 = nextOperand(ip);
                a2 = nextOperand(ip);
                storage[a1] = a2;
                break;
            case opIncrement:
                a1 = pop();
                push(a1+1);
                break;
            case opDecrement:
                a1 = pop();
                push(a1-1);
                break;
            case opFetch:
                a1 = nextOperand(ip);
                push(fetch(a1));
                break;

            case opAddItems:
                a1 = nextOperand(ip);
                a2 = nextOperand(ip);
                for (int i = 0; i < inventory.size(); ++i) {
                    if (inventory[i].itemIdent == a2) {
                        inventory[i].qty += a1;
                        a1 = 0;
                        break;
                    }
                }
                if (a1 > 0) {
                    inventory.push_back(CarriedItem(a1, a2));
                }
                break;
            case opRemoveItems:
                a1 = nextOperand(ip);
                a2 = nextOperand(ip);
                for (auto ci = inventory.begin(); ci != inventory.end(); ++ci) {
                    if (ci->itemIdent == a2) {
                        ci->qty -= a1;
                        if (ci->qty <= 0) {
                            inventory.erase(ci);
                            break;
                        }
                    }
                }
                break;
            default: {
                std::stringstream ss;
                ss << std::hex;
                ss << "Encountered unknown command 0x" << cmdCode;
                ss << " at 0x" << (ip-1) << '.';
                throw PlayError(ss.str());
            }
        }
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

bool Game::actionAllowed() const {
    return inLocation && isRunning;
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
