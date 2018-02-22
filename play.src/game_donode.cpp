#include <iomanip>
#include <sstream>

#include "play.h"

class Stack {
public:
    void push(std::uint32_t value) {
        mStack.push_back(value);
    }
    std::uint32_t pop() {
        std::uint32_t value = mStack.back();
        mStack.pop_back();
        return value;
    }
    std::uint32_t peek(unsigned position = 0) {
        if (position >= mStack.size()) {
            throw std::runtime_error("stack index out of bounds");
        }

        return mStack[mStack.size() - 1 - position];
    }
    void swap(unsigned pos1 = 0, unsigned pos2 = 1) {
        if (pos1 >= mStack.size() || pos2 >= mStack.size()) {
            throw std::runtime_error("stack index out of bounds");
        }

        std::uint32_t value = mStack[pos1];
        mStack[pos1] = mStack[pos2];
        mStack[pos2] = value;
    }

    bool isEmpty() const {
        return mStack.empty();
    }
    unsigned size() const {
        return mStack.size();
    }
private:
    std::vector<std::uint32_t> mStack;
};

std::uint32_t Game::doNode(std::uint32_t address) {
    std::uint32_t ip = address;
    if (!isType(ip++, idNode)) {
        std::stringstream ss;
        ss << "Tried to run non-node at " << std::hex << std::uppercase << (int)readByte(ip-1) << ".";
        throw PlayError(ss.str());
    }
    Stack stack;

    std::uint32_t a1, a2, a3, a4;
    while (true) {
        std::uint8_t cmdCode = readByte(ip++);

        switch(cmdCode) {
            case opEnd:
                if (stack.isEmpty()) {
                    return 0;
                } else {
                    return stack.pop();
                }
            case opDoNode:
                a1 = call(stack.pop(), false, false);
                stack.push(a1);
                break;
            case opStartGame: // start-game;
                gameStarted = true;
                break;
            case opAddTime: // add-time [hours] [minutes];
                a2 = stack.pop();
                a1 = stack.pop();
                gameTime += a1 * minutesPerHour + a2;
                break;
            case opPush:
                a1 = readWord(ip);
                ip += 4;
                stack.push(a1);
                break;
            case opPop:
                stack.pop();
                break;

            case opAddOption:
                a2 = stack.pop();
                a1 = stack.pop();
                options.push_back(Option(a1, a2));
                break;
            case opAddOptionXtra:
                a3 = stack.pop();
                a2 = stack.pop();
                a1 = stack.pop();
                options.push_back(Option(a1, a2, a3));
                break;
            case opAddContinue:
                a1 = stack.pop();
                options.push_back(Option(1, a1));
                break;
            case opAddReturn:
                options.push_back(Option(1, location));
                break;

            case opSay:
                say(getNameOf(stack.pop()));
                break;
            case opSayUF:
                say(toUpperFirst(getNameOf(stack.pop())));
                break;
            case opSayTC:
                say(toTitleCase(getNameOf(stack.pop())));
                break;
            case opSayPronoun:
                a2 = stack.pop();
                a1 = stack.pop();
                say(getPronoun(a1, a2));
                break;
            case opSayPronounUF:
                a2 = stack.pop();
                a1 = stack.pop();
                say(toUpperFirst(getPronoun(a1, a2)));
                break;
            case opSayNumber:
                say(stack.pop());
                break;

            case opJump:
                ip = stack.pop();
                break;
            case opJumpTrue:
                a2 = stack.pop();
                a1 = stack.pop();
                if (a1) {
                    ip = a2;
                }
                break;
            case opJumpFalse:
                a2 = stack.pop();
                a1 = stack.pop();
                if (!a1) {
                    ip = a2;
                }
                break;
            case opJumpEq:
                a3 = stack.pop();
                a1 = stack.pop();
                a2 = stack.pop();
                if (a1 == a2) {
                    ip = a3;
                }
                break;
            case opJumpNeq:
                a3 = stack.pop();
                a1 = stack.pop();
                a2 = stack.pop();
                if (a1 != a2) {
                    ip = a3;
                }
                break;
            case opJumpLt:
                a3 = stack.pop();
                a1 = stack.pop();
                a2 = stack.pop();
                if (static_cast<int>(a1) > static_cast<int>(a2)) {
                    ip = a3;
                }
                break;
            case opJumpLte:
                a3 = stack.pop();
                a1 = stack.pop();
                a2 = stack.pop();
                if (static_cast<int>(a1) >= static_cast<int>(a2)) {
                    ip = a3;
                }
                break;
            case opJumpGt:
                a3 = stack.pop();
                a1 = stack.pop();
                a2 = stack.pop();
                if (static_cast<int>(a1) < static_cast<int>(a2)) {
                    ip = a3;
                }
                break;
            case opJumpGte:
                a3 = stack.pop();
                a1 = stack.pop();
                a2 = stack.pop();
                if (static_cast<int>(a1) <= static_cast<int>(a2)) {
                    ip = a3;
                }
                break;

            case opStore:
                a2 = stack.pop();
                a1 = stack.pop();
                if (a2) {
                    storage[a1] = a2;
                } else {
                    storage.erase(a1);
                }
                break;
            case opFetch:
                stack.push(fetch(stack.pop()));
                break;

            case opAddItems:
                a2 = stack.pop(); // qty
                a1 = stack.pop(); // itemIdent
                stack.push(addItems(a2, a1));
                break;
            case opRemoveItems:
                a2 = stack.pop(); // qty
                a1 = stack.pop(); // itemIDent
                stack.push(removeItems(a2, a1));
                break;
            case opItemQty:
                a1 = stack.pop(); // itemIdent
                stack.push(itemQty(a1));
                break;

            case opAddToList: {
                a1 = stack.pop(); // item to add
                a2 = stack.pop(); // list ident
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
                a1 = stack.pop(); // item to remove
                a2 = stack.pop(); // list ident
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
                a1 = stack.pop(); // item to check for
                a2 = stack.pop(); // list ident
                std::shared_ptr<List> list = getList(a3);
                if (!list) {
                    stack.push(false);
                } else {
                    stack.push(list->contains(a1));
                }
                break;
            }
            case opRandomFromList: {
                a1 = stack.pop(); // list ident
                std::shared_ptr<List> list = getList(a1);
                if (list) {
                    stack.push(list->random());
                } else {
                    std::stringstream ss;
                    ss << "Tried to get a random item from non-existant list " << a1 << '.';
                    throw PlayError(ss.str());
                }
                break;
            }
            case opCreateList: {
                a1 = stack.pop(); // location to store list
                List *list = new List;
                std::uint32_t ident = nextListIdent++;
                addList(ident, std::shared_ptr<List>(list));
                storage[a1] = ident;
                break;
            }
            case opAddToListChance: {
                a2 = stack.pop(); // item chance
                a1 = stack.pop(); // item to add
                a3 = stack.pop(); // list ident
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
                resetCharacter(stack.pop());
                break;
            case opGetSex: {
                Character *c = getCharacter(stack.pop());
                stack.push(c->sex);
                break;
            }
            case opGetSpecies: {
                Character *c = getCharacter(stack.pop());
                stack.push(c->species);
                break;
            }
            case opSetSex: {
                a1 = stack.pop();
                a2 = stack.pop();
                if (getObjectProperty(a1, propClass) != ocSex) {
                    throw PlayError("Tried to set character sex to non-sex.");
                }
                Character *c = getCharacter(a2);
                c->sex = a1;
                break;
            }
            case opSetSpecies: {
                a1 = stack.pop();
                a2 = stack.pop();
                if (getObjectProperty(a1, propClass) != ocSpecies) {
                    throw PlayError("Tried to set character species to non-species.");
                }
                Character *c = getCharacter(a2);
                c->species = a1;
                break;
            }
            case opGetSkill:
                a1 = stack.pop();
                a2 = stack.pop();
                stack.push(getSkillMax(a2, a1));
                break;
            case opAdjSkill:
                a1 = stack.pop();
                a2 = stack.pop();
                a3 = stack.pop();
                adjSkillMax(a3, a2, a1);
                break;
            case opGetSkillCur:
                a1 = stack.pop();
                a2 = stack.pop();
                stack.push(getSkillCur(a2, a1));
                break;
            case opAdjSkillCur:
                a1 = stack.pop();
                a2 = stack.pop();
                a3 = stack.pop();
                adjSkillCur(a3, a2, a1);
                break;
            case opSkillCheck:
                a1 = stack.pop(); // target
                a2 = stack.pop(); // modifier
                a3 = stack.pop(); // skill
                a4 = stack.pop(); // character
                stack.push(doSkillCheck(a4, a3, a2, a1));
                break;
            case opDoDamage:
                a1 = stack.pop();
                a2 = stack.pop();
                a3 = stack.pop();
                a4 = stack.pop();
                doDamage(a4, a3, a2, a1);
                break;

            case opAdd:
                stack.push(stack.pop()+stack.pop());
                break;
            case opSubtract:
                stack.push(stack.pop()-stack.pop());
                break;
            case opMultiply:
                stack.push(stack.pop()*stack.pop());
                break;
            case opDivide:
                stack.push(stack.pop()/stack.pop());
                break;
            case opModulo:
                stack.push(stack.pop()%stack.pop());
                break;
            case opPower:
                a1 = stack.pop();
                a2 = stack.pop();
                a3 = 1;
                for (unsigned i = 0; i < a2; ++i) {
                    a3 *= a1;
                }
                stack.push(a3);
                break;
            case opIncrement:
                stack.push(stack.pop()+1);
                break;
            case opDecrement:
                stack.push(stack.pop()-1);
                break;

            case opAddToParty:
                party.push_back(stack.pop());
                break;
            case opIsInParty: {
                a1 = stack.pop();
                bool found = false;
                for (unsigned i = 0; i < party.size(); ++i) {
                    if (party[i] == a1) {
                        found = true;
                    }
                }
                stack.push(found);
                break;
            }
            case opRemoveFromParty: {
                a1 = stack.pop();
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
                afterCombatNode = stack.pop();
                inCombat = startedCombat = true;
                combatants.clear();
                currentCombatant = 0;
                combatRound = 1;
                for (const auto &partyMember : party) {
                    combatants.push_back(partyMember);
                }
                break;
            case opAddToCombat:
                a1 = stack.pop();
                restoreCharacter(a1);
                combatants.push_back(a1);
                break;
            case opCombatant:
                a1 = stack.pop();
                if (a1 >= combatants.size()) {
                    stack.push(0);
                } else {
                    stack.push(combatants[a1]);
                }
                break;

            case opGetProperty: {
                a2 = stack.pop();
                a1 = stack.pop();
                stack.push(getObjectProperty(a1, a2));
                break; }

            case opRandomOfFaction: {
                if (!inCombat) break;
                a1 = stack.pop();
                std::vector<std::uint32_t> options;
                for (std::uint32_t who : combatants) {
                    if (getObjectProperty(who, propFaction) == a1) {
                        options.push_back(who);
                    }
                }
                if (options.empty()) {
                    stack.push(0);
                } else {
                    stack.push(options[rand() % options.size()]);
                }
                break; }
            case opRandomNotFaction: {
                if (!inCombat) break;
                a1 = stack.pop();
                std::vector<std::uint32_t> options;
                for (std::uint32_t who : combatants) {
                    if (getObjectProperty(who, propFaction) != a1) {
                        options.push_back(who);
                    }
                }
                if (options.empty()) {
                    stack.push(0);
                } else {
                    stack.push(options[rand() % options.size()]);
                }
                break; }

            case opStackSwap:
                stack.swap();
                break;
            case opStackDup:
                stack.push(stack.peek());
                break;
            case opStackCount:
                stack.push(stack.size());
                break;

            case opIsKOed: {
                stack.push(isKOed(stack.pop()));
                break; }
            case opHasProperty:
                a2 = stack.pop();
                a1 = stack.pop();
                stack.push(objectHasProperty(a1, a2));
                break;
            case opPartySize:
                stack.push(party.size());
                break;
            case opPartyIsKOed: {
                bool partyIsKOed = true;
                for (unsigned i = 0; i < party.size(); ++i) {
                    if (!isKOed(party[i])) {
                        partyIsKOed = false;
                        break;
                    }
                }
                stack.push(partyIsKOed);
                break;}
            case opDoRest:
                doRest(stack.pop());
                break;
            case opCombatStatus:
                stack.push(combatStatus());
                break;
            case opPartyAt:
                a1 = stack.pop();
                if (a1 >= party.size()) {
                    stack.push(0);
                } else {
                    stack.push(party[a1]);
                }
                break;
            case opGetEquip: {
                a1 = stack.pop(); // slot
                a2 = stack.pop(); // whoRef
                Character *who = getCharacter(a2);
                if (!who) {
                    throw PlayError("Tried to get equipment on non-character");
                }
                if (who->gear.count(a1) > 0) {
                    stack.push(who->gear[a1]);
                } else {
                    stack.push(0);
                }
                break; }
            case opSetEquip: {
                a1 = stack.pop(); // itemRef
                a2 = stack.pop(); // whoRef
                Character *who = getCharacter(a2);
                if (!who) {
                    throw PlayError("Tried to set equipment on non-character");
                }
                if (getObjectProperty(a1, propClass) != ocItem) {
                    throw PlayError("Tried to equip non-item");
                }
                std::uint32_t slot = getObjectProperty(a1, propSlot);
                if (slot == 0) {
                    throw PlayError("Tried to equip non-equippable item");
                }
                who->gear[slot] = a1;
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