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

    std::uint32_t operands[4], a1, a2, a3, a4;
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
                doNode(pop());
                break;
            case opSetLocation:
                a1 = pop();
                inLocation = true;
                if (locationName != a1) {
                    newLocation = true;
                    locationName = a1;
                    location = address;
                }
                break;
            case opStartGame: // start-game;
                gameStarted = true;
                break;
            case opAddTime: // add-time [hours] [minutes];
                a2 = pop();
                a1 = pop();
                gameTime += a1 * minutesPerHour + a2;
                break;
            case opPush:
                a1 = operands[0];
                push(a1);
                break;
            case opPop:
                pop();
                break;

            case opAddOption:
                a2 = pop();
                a1 = pop();
                options.push_back(Option(a1, a2));
                break;
            case opAddOptionXtra:
                a3 = pop();
                a2 = pop();
                a1 = pop();
                options.push_back(Option(a1, a2, a3));
                break;
            case opAddContinue:
                a1 = pop();
                options.push_back(Option(1, a1));
                break;
            case opAddReturn:
                options.push_back(Option(1, location));
                break;

            case opSay:
                say(getNameOf(pop()));
                break;
            case opSayUF:
                say(toUpperFirst(getNameOf(pop())));
                break;
            case opSayTC:
                say(toTitleCase(getNameOf(pop())));
                break;
            case opSayPronoun:
                a2 = pop();
                a1 = pop();
                say(getPronoun(a1, a2));
                break;
            case opSayPronounUF:
                a2 = pop();
                a1 = pop();
                say(toUpperFirst(getPronoun(a1, a2)));
                break;
            case opSayNumber:
                say(pop());
                break;

            case opJump:
                ip = pop();
                break;
            case opJumpTrue:
                a2 = pop();
                a1 = pop();
                if (a1) {
                    ip = a2;
                }
                break;
            case opJumpFalse:
                a2 = pop();
                a1 = pop();
                if (!a1) {
                    ip = a2;
                }
                break;
            case opJumpEq:
                a3 = pop();
                a1 = pop();
                a2 = pop();
                if (a1 == a2) {
                    ip = a3;
                }
                break;
            case opJumpNeq:
                a3 = pop();
                a1 = pop();
                a2 = pop();
                if (a1 != a2) {
                    ip = a3;
                }
                break;
            case opJumpLt:
                a3 = pop();
                a1 = pop();
                a2 = pop();
                if (a1 < a2) {
                    ip = a3;
                }
                break;
            case opJumpLte:
                a3 = pop();
                a1 = pop();
                a2 = pop();
                if (a1 <= a2) {
                    ip = a3;
                }
                break;
            case opJumpGt:
                a3 = pop();
                a1 = pop();
                a2 = pop();
                if (a1 > a2) {
                    ip = a3;
                }
                break;
            case opJumpGte:
                a3 = pop();
                a1 = pop();
                a2 = pop();
                if (a1 >= a2) {
                    ip = a3;
                }
                break;

            case opStore:
                a2 = pop();
                a1 = pop();
                if (a2) {
                    storage[a1] = a2;
                } else {
                    storage.erase(a1);
                }
                break;
            case opFetch:
                push(fetch(pop()));
                break;

            case opAddItems:
                a2 = pop(); // qty
                a1 = pop(); // itemIdent
                push(addItems(a2, a1));
                break;
            case opRemoveItems:
                a2 = pop(); // qty
                a1 = pop(); // itemIDent
                push(removeItems(a2, a1));
                break;
            case opItemQty:
                a1 = pop(); // itemIdent
                push(itemQty(a1));
                break;

            case opAddToList: {
                a1 = pop(); // item to add
                a2 = pop(); // list ident
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
                a1 = pop(); // item to remove
                a2 = pop(); // list ident
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
                a1 = pop(); // item to check for
                a2 = pop(); // list ident
                std::shared_ptr<List> list = getList(a3);
                if (!list) {
                    push(false);
                } else {
                    push(list->contains(a1));
                }
                break;
            }
            case opRandomFromList: {
                a1 = pop(); // list ident
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
                a1 = pop(); // location to store list
                List *list = new List;
                std::uint32_t ident = nextListIdent++;
                addList(ident, std::shared_ptr<List>(list));
                storage[a1] = ident;
                break;
            }
            case opAddToListChance: {
                a2 = pop(); // item chance
                a1 = pop(); // item to add
                a3 = pop(); // list ident
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
                resetCharacter(pop());
                break;
            case opGetSex: {
                Character *c = getCharacter(pop());
                push(c->sex);
                break;
            }
            case opGetSpecies: {
                Character *c = getCharacter(pop());
                push(c->species);
                break;
            }
            case opSetSex: {
                a1 = pop();
                a2 = pop();
                if (getObjectProperty(a1, propClass) != ocSex) {
                    throw PlayError("Tried to set character sex to non-sex.");
                }
                Character *c = getCharacter(a2);
                c->sex = a1;
                break;
            }
            case opSetSpecies: {
                a1 = pop();
                a2 = pop();
                if (getObjectProperty(a1, propClass) != ocSpecies) {
                    throw PlayError("Tried to set character species to non-species.");
                }
                Character *c = getCharacter(a2);
                c->species = a1;
                break;
            }
            case opGetSkill:
                a1 = pop();
                a2 = pop();
                push(getSkillMax(a2, a1));
                break;
            case opAdjSkill:
                a1 = pop();
                a2 = pop();
                a3 = pop();
                adjSkillMax(a3, a2, a1);
                break;
            case opGetSkillCur:
                a1 = pop();
                a2 = pop();
                push(getSkillCur(a2, a1));
                break;
            case opAdjSkillCur:
                a1 = pop();
                a2 = pop();
                a3 = pop();
                adjSkillCur(a3, a2, a1);
                break;
            case opSkillCheck:
                a1 = pop(); // target
                a2 = pop(); // modifier
                a3 = pop(); // skill
                a4 = pop(); // character
                push(doSkillCheck(a4, a3, a2, a1));
                break;
            case opDoDamage:
                a1 = pop();
                a2 = pop();
                a3 = pop();
                a4 = pop();
                doDamage(a4, a3, a2, a1);
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
                party.push_back(pop());
                break;
            case opIsInParty: {
                a1 = pop();
                bool found = false;
                for (unsigned i = 0; i < party.size(); ++i) {
                    if (party[i] == a1) {
                        found = true;
                    }
                }
                push(found);
                break;
            }
            case opRemoveFromParty: {
                a1 = pop();
                auto i = party.begin();
                while (i != party.end()) {
                    if (*i == a1) {
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
                combatants.push_back(pop());
                break;
            case opCombatant:
                a1 = pop();
                if (a1 >= combatants.size()) {
                    push(0);
                } else {
                    push(combatants[a1]);
                }
                break;

            case opGetProperty: {
                a2 = pop();
                a1 = pop();
                push(getObjectProperty(a1, a2));
                break; }

            case opRandomOfFaction: {
                if (!inCombat) break;
                a1 = pop();
                std::vector<std::uint32_t> options;
                for (std::uint32_t who : combatants) {
                    if (getObjectProperty(who, propFaction) == a1) {
                        options.push_back(who);
                    }
                }
                if (options.empty()) {
                    push(0);
                } else {
                    push(options[rand() % options.size()]);
                }
                break; }
            case opRandomNotFaction: {
                if (!inCombat) break;
                a1 = pop();
                std::vector<std::uint32_t> options;
                for (std::uint32_t who : combatants) {
                    if (getObjectProperty(who, propFaction) != a1) {
                        options.push_back(who);
                    }
                }
                if (options.empty()) {
                    push(0);
                } else {
                    push(options[rand() % options.size()]);
                }
                break; }

            case opStackSwap:
                a1 = pop();
                a2 = pop();
                push(a1);
                push(a2);
                break;
            case opStackDup:
                a1 = pop();
                push(a1);
                push(a1);
                break;
            case opStackCount:
                push(stack.size());
                break;

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