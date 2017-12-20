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
    io.say("\n");
}

void Game::doNode(std::uint32_t address) {
    std::uint32_t ip = address;
    if (readByte(ip++) != idNode) {
        std::stringstream ss;
        ss << "Tried to run non-node at " << std::hex << (int)readByte(ip-1) << ".";
        throw PlayError(ss.str());
    }

/*
const int opJump            = 0x30;
const int opJumpTrue        = 0x31;
const int opJumpFalse       = 0x32;
const int opJumpEq          = 0x33;
const int opJumpNeq         = 0x34;
const int opJumpLt          = 0x35;
const int opJumpLte         = 0x36;
const int opJumpGt          = 0x37;
const int opJumpGte         = 0x38;
const int opStore           = 0x40;
const int opIncrement       = 0x41;
const int opDecrement       = 0x42;
const int opForget          = 0x43;
*/
    std::uint32_t a1, a2, a3;
    while (true) {
        std::uint8_t cmdCode = readByte(ip++);
        switch(cmdCode) {
            case opEnd:
                return;
            case opDoNode:
                doNode(nextWord(ip));
                break;
            case opSetLocation:
                a1 = nextWord(ip);
                inLocation = true;
                if (locationName != a1) {
                    newLocation = true;
                    locationName = a1;
                    location = address;
                }
                break;
            case opAddOption:
                a1 = nextWord(ip);
                a2 = nextWord(ip);
                options.push_back(Option(a1, a2));
                break;
            case opAddOptionXtra:
                a1 = nextWord(ip);
                a2 = nextWord(ip);
                a3 = nextWord(ip);
                options.push_back(Option(a1, a2, a3));
                break;
            case opAddContinue:
                a1 = nextWord(ip);
                options.push_back(Option(1, a1));
                break;
            case opAddReturn:
                options.push_back(Option(1, location));
                break;
            case opSay:
                io.say(getString(nextWord(ip)));
                break;
            case opJump:
                ip = nextWord(ip);
                break;
            case opJumpTrue:
                a1 = nextWord(ip);
                a2 = nextWord(ip);
                if (storage.count(a1) != 0 && storage[a1]) {
                    ip = a2;
                }
                break;
            case opJumpFalse:
                a1 = nextWord(ip);
                a2 = nextWord(ip);
                if (storage.count(a1) == 0 || !storage[a1]) {
                    ip = a2;
                }
                break;
            case opStore:
                a1 = nextWord(ip);
                a2 = nextWord(ip);
                storage[a1] = a2;
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
