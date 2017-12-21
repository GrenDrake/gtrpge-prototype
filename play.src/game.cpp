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
void Game::doNode(std::uint32_t address) {
    std::uint32_t ip = address;
    if (!isType(ip++, idNode)) {
        std::stringstream ss;
        ss << "Tried to run non-node at " << std::hex << std::uppercase << (int)readByte(ip-1) << ".";
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
            case opJumpEq:
                a1 = pop();
                a2 = nextOperand(ip);
                a3 = nextOperand(ip);
                if (a1 == a2) {
                    ip = a3;
                }
                break;
            case opJumpNeq:
                a1 = pop();
                a2 = nextOperand(ip);
                a3 = nextOperand(ip);
                if (a1 != a2) {
                    ip = a3;
                }
                break;
            case opJumpLt:
                a1 = pop();
                a2 = nextOperand(ip);
                a3 = nextOperand(ip);
                if (a1 < a2) {
                    ip = a3;
                }
                break;
            case opJumpLte:
                a1 = pop();
                a2 = nextOperand(ip);
                a3 = nextOperand(ip);
                if (a1 <= a2) {
                    ip = a3;
                }
                break;
            case opJumpGt:
                a1 = pop();
                a2 = nextOperand(ip);
                a3 = nextOperand(ip);
                if (a1 > a2) {
                    ip = a3;
                }
                break;
            case opJumpGte:
                a1 = pop();
                a2 = nextOperand(ip);
                a3 = nextOperand(ip);
                if (a1 >= a2) {
                    ip = a3;
                }
                break;

            case opStore:
                a1 = nextOperand(ip);
                a2 = nextOperand(ip);
                if (a2) {
                    storage[a1] = a2;
                } else {
                    storage.erase(a1);
                }
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
                a1 = nextOperand(ip); // qty
                a2 = nextOperand(ip); // itemIdent
                for (auto ci = inventory.begin(); ci != inventory.end(); ++ci) {
                    if (ci->itemIdent == a2) {
                        if (ci->qty >= a1) {
                            ci->qty -= a1;
                            if (ci->qty <= 0) {
                                inventory.erase(ci);
                                break;
                            }
                            a1 = 0;
                        }
                    }
                }
                push(a1 == 0);
                break;
            case opItemQty:
                a1 = nextOperand(ip); // itemIdent
                a2 = 0;
                for (CarriedItem &ci : inventory) {
                    if (ci.itemIdent == a1) {
                        a2 = ci.qty;
                        break;
                    }
                }
                push(a2);
                break;

            case opIncrement:
                a1 = pop();
                push(a1+1);
                break;
            case opDecrement:
                a1 = pop();
                push(a1-1);
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
