#include <iomanip>
#include <sstream>

#include "play.h"


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
            case opHasFlag:
                a1 = nextOperand(ip);
                a2 = nextOperand(ip);
                push(hasFlag(a1, a2));
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
                a1 = nextOperand(ip);
                sayAddress(a1);
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
                push(addItems(a1, a2));
                break;
            case opRemoveItems:
                a1 = nextOperand(ip); // qty
                a2 = nextOperand(ip); // itemIdent
                push(removeItems(a1, a2));
                break;
            case opItemQty:
                a1 = nextOperand(ip); // itemIdent
                push(itemQty(a1));
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