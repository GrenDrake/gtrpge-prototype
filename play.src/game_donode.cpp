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

    std::uint32_t operands[4], a1, a2, a3;
    while (true) {
        std::uint8_t cmdCode = readByte(ip++);
        std::uint8_t operandTypesByte = readByte(ip++);
        for (unsigned i = 0; i < 4; ++i) {
            int type = operandTypesByte >> (i * 2);
            type &= 0x03;
            switch(type) {
                case operandNone:
                    operands[i] = 0;
                    break;
                case operandStack:
                    operands[i] = pop();
                    break;
                case operandStorage:
                    a1 = readWord(ip);
                    operands[i] = fetch(a1);
                    ip += 4;
                    break;
                case operandImmediate:
                    operands[i] = readWord(ip);
                    ip += 4;
                    break;
            }
        }

        switch(cmdCode) {
            case opEnd:
                return;
            case opDoNode:
                doNode(operands[0]);
                break;
            case opSetLocation:
                a1 = operands[0];
                inLocation = true;
                if (locationName != a1) {
                    newLocation = true;
                    locationName = a1;
                    location = address;
                }
                break;
            case opHasFlag:
                a1 = operands[0];
                a2 = operands[1];
                push(hasFlag(a1, a2));
                break;
            case opStartGame: // start-game;
                gameStarted = true;
                break;
            case opAddTime: // add-time [hours] [minutes];
                gameTime += operands[0] * minutesPerHour + operands[1];
                break;
            case opPush:
                a1 = operands[0];
                push(a1);
                break;
            case opPop:
                pop();
                break;

            case opAddOption:
                a1 = operands[0];
                a2 = operands[1];
                options.push_back(Option(a1, a2));
                break;
            case opAddOptionXtra:
                a1 = operands[0];
                a2 = operands[1];
                a3 = operands[2];
                options.push_back(Option(a1, a2, a3));
                break;
            case opAddContinue:
                a1 = operands[1];
                options.push_back(Option(1, a1));
                break;
            case opAddReturn:
                options.push_back(Option(1, location));
                break;

            case opSay:
                say(getNameOf(operands[0]));
                break;
            case opSayUF:
                say(toUpperFirst(getNameOf(operands[0])));
                break;
            case opSayTC:
                say(toTitleCase(getNameOf(operands[0])));
                break;
            case opSayPronoun:
                say(getPronoun(operands[0], operands[1]));
                break;
            case opSayPronounUF:
                say(toUpperFirst(getPronoun(operands[0], operands[1])));
                break;
            case opSayNumber:
                say(operands[0]);
                break;

            case opJump:
                ip = operands[0];
                break;
            case opJumpTrue:
                a1 = pop();
                a2 = operands[0];
                if (a1) {
                    ip = a2;
                }
                break;
            case opJumpFalse:
                a1 = pop();
                a2 = operands[0];
                if (!a1) {
                    ip = a2;
                }
                break;
            case opJumpEq:
                a1 = pop();
                a2 = operands[0];
                a3 = operands[1];
                if (a1 == a2) {
                    ip = a3;
                }
                break;
            case opJumpNeq:
                a1 = pop();
                a2 = operands[0];
                a3 = operands[1];
                if (a1 != a2) {
                    ip = a3;
                }
                break;
            case opJumpLt:
                a1 = pop();
                a2 = operands[0];
                a3 = operands[1];
                if (a1 < a2) {
                    ip = a3;
                }
                break;
            case opJumpLte:
                a1 = pop();
                a2 = operands[0];
                a3 = operands[1];
                if (a1 <= a2) {
                    ip = a3;
                }
                break;
            case opJumpGt:
                a1 = pop();
                a2 = operands[0];
                a3 = operands[1];
                if (a1 > a2) {
                    ip = a3;
                }
                break;
            case opJumpGte:
                a1 = pop();
                a2 = operands[0];
                a3 = operands[1];
                if (a1 >= a2) {
                    ip = a3;
                }
                break;

            case opStore:
                a1 = operands[0];
                a2 = operands[1];
                if (a2) {
                    storage[a1] = a2;
                } else {
                    storage.erase(a1);
                }
                break;
            case opFetch:
                a1 = operands[0];
                push(fetch(a1));
                break;

            case opAddItems:
                a1 = operands[0];
                a2 = operands[1];
                push(addItems(a1, a2));
                break;
            case opRemoveItems:
                a1 = operands[0]; // qty
                a2 = operands[1]; // itemIdent
                push(removeItems(a1, a2));
                break;
            case opItemQty:
                a1 = operands[0]; // itemIdent
                push(itemQty(a1));
                break;

            case opAddToList: {
                a1 = operands[0]; // item to add
                a2 = operands[1]; // list ident
                std::shared_ptr<List> list = getList(a2);
                if (list) {
                    list->add(a1, 1);
                } else {
                    std::stringstream ss;
                    ss << "Tried to add to non-existant list " << a2 << '.';
                    throw PlayError(ss.str());
                }
                break;
            }
            case opRemoveFromList: {
                a1 = operands[0]; // item to remove
                a2 = operands[1]; // list ident
                std::shared_ptr<List> list = getList(a2);
                if (list) {
                    list->remove(a1);
                } else {
                    std::stringstream ss;
                    ss << "Tried to remove from non-existant list " << a2 << '.';
                    throw PlayError(ss.str());
                }
                break;
            }
            case opIsInList: {
                a1 = operands[0]; // item to check for
                a2 = operands[1]; // list ident
                std::shared_ptr<List> list = getList(a3);
                if (!list) {
                    push(false);
                } else {
                    push(list->contains(a1));
                }
                break;
            }
            case opRandomFromList: {
                a1 = operands[0]; // list ident
                std::shared_ptr<List> list = getList(a1);
                if (list) {
                    push(list->random());
                } else {
                    std::stringstream ss;
                    ss << "Tried to get a random item from non-existant list " << a1 << '.';
                    throw PlayError(ss.str());
                }
                break;
            }
            case opCreateList: {
                a1 = operands[0]; // location to store list
                List *list = new List;
                std::uint32_t ident = nextListIdent++;
                addList(ident, std::shared_ptr<List>(list));
                storage[a1] = ident;
                break;
            }
            case opAddToListChance: {
                a1 = operands[0]; // item to add
                a2 = operands[1]; // item chance
                a3 = operands[2]; // list ident
                std::shared_ptr<List> list = getList(a3);
                if (list) {
                    list->add(a1, a2);
                } else {
                    std::stringstream ss;
                    ss << "Tried to add to non-existant list " << a3 << '.';
                    throw PlayError(ss.str());
                }
                break;
            }

            case opResetCharacter:
                resetCharacter(operands[0]);
                break;
            case opGetSex: {
                Character *c = getCharacter(operands[0]);
                push(c->sex);
                break;
            }
            case opGetSpecies: {
                Character *c = getCharacter(operands[0]);
                push(c->species);
                break;
            }
            case opSetSex: {
                if (!isType(operands[1], idSex)) {
                    throw PlayError("Tried to set character sex to non-sex.");
                }
                Character *c = getCharacter(operands[0]);
                c->sex = operands[1];
                break;
            }
            case opSetSpecies: {
                if (!isType(operands[1], idSpecies)) {
                    throw PlayError("Tried to set character species to non-species.");
                }
                Character *c = getCharacter(operands[0]);
                c->species = operands[1];
                break;
            }
            case opGetSkill:
                push(getSkillMax(operands[0], operands[1]));
                break;
            case opAdjSkill:
                adjSkillMax(operands[0], operands[1], operands[2]);
                break;
            case opGetSkillCur:
                push(getSkillCur(operands[0], operands[1]));
                break;
            case opAdjSkillCur:
                adjSkillCur(operands[0], operands[1], operands[2]);
                break;
            case opSkillCheck:
                push(doSkillCheck(operands[0], operands[1], operands[2], operands[3]));
                break;
            case opDoDamage:
                doDamage(operands[0], operands[1], operands[2], operands[3]);
                break;

            case opAdd:
                push(pop()+pop());
                break;
            case opSubtract:
                push(pop()-pop());
                break;
            case opMultiply:
                push(pop()*pop());
                break;
            case opDivide:
                push(pop()/pop());
                break;
            case opModulo:
                push(pop()%pop());
                break;
            case opPower:
                a1 = pop();
                a2 = pop();
                a3 = 1;
                for (unsigned i = 0; i < a2; ++i) {
                    a3 *= a1;
                }
                push(a3);
                break;
            case opIncrement:
                push(pop()+1);
                break;
            case opDecrement:
                push(pop()-1);
                break;

            case opAddToParty:
                party.push_back(operands[0]);
                break;
            case opIsInParty: {
                bool found = false;
                for (unsigned i = 0; i < party.size(); ++i) {
                    if (party[i] == operands[0]) {
                        found = true;
                    }
                }
                push(found);
                break;
            }
            case opRemoveFromParty: {
                auto i = party.begin();
                while (i != party.end()) {
                    if (*i == operands[0]) {
                        i = party.erase(i);
                    } else {
                        ++i;
                    }
                }
                break;
            }

            case opResetCombat:
                inCombat = startedCombat = true;
                combatants.clear();
                currentCombatant = 0;
                combatRound = 1;
                for (const auto &partyMember : party) {
                    combatants.push_back(partyMember);
                }
                break;
            case opAddToCombat:
                combatants.push_back(operands[0]);
                break;

            case opGetProperty: {
                push(getObjectProperty(operands[0], operands[1]));
                break; }

            default: {
                std::stringstream ss;
                ss << std::hex;
                ss << "Encountered unknown command 0x" << (int)cmdCode;
                ss << " at 0x" << (ip-1) << '.';
                throw PlayError(ss.str());
            }
        }
    }
}