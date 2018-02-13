#include <algorithm>
#include <cctype>
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


int Game::roll(int dice, int sides) {
    int result = 0;
    for (int i = 0; i < dice; ++i) {
        result += 1 + rand() % sides;
    }
    return result;
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

    doGameSetup();
}

void Game::setDataAs(uint8_t *data, size_t size) {
    uint8_t *dataCopy = static_cast<uint8_t*>(malloc(size));
    memcpy(dataCopy, data, size);
    this->data = dataCopy;
    this->dataSize = size;

    doGameSetup();
}


void Game::clearOutput() {
    outputBuffer = "";
}

std::string Game::getOutput() const {
    std::string text = outputBuffer;
    auto new_end =
        std::unique(text.begin(), text.end(),
            [](char lhs, char rhs){ return (lhs == rhs) && (lhs == ' ' || lhs == '\n'); }
        );
    text.erase(new_end, text.end());
    return trim(text);
}

std::string Game::getTimeString(bool exact) {
    std::stringstream ss;

    int minutes = gameTime;

    int hour = minutes / minutesPerHour;
    minutes -= hour * minutesPerHour;

    int day = hour / hoursPerDay;
    hour -= day * hoursPerDay;

    if (exact) {
        if (hour <= 12) {
            if (hour == 0)  hour = 12;
            ss << hour << ":" << std::setw(2) << std::setfill('0') << minutes << " AM";
        } else {
            ss << (hour-12) << ":" << std::setw(2) << std::setfill('0') << minutes << " PM";
        }
    } else {
        if (hour <  6)      ss << "night";
        else if (hour < 12) ss << "morning";
        else if (hour < 13) ss << "midday";
        else if (hour < 18) ss << "afternoon";
        else if (hour < 22) ss << "evening";
        else                ss << "night";
    }
    ss << " on Day " << (day+1);

    return ss.str();
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

std::uint32_t Game::getObjectProperty(std::uint32_t objRef, std::uint16_t propId) {
    if (!isType(objRef, idObject)) {
        throw PlayError("Tried to get property of non-object");
    }

    const std::uint16_t propCount = readShort(objRef + 1);
    std::uint32_t propValue = 0;
    for (std::uint16_t i = 0; i < propCount; ++i) {
        std::uint16_t idHere = readShort(objRef + 3 + i * 6);
        if (idHere == propId) {
            propValue = readWord(objRef + 5 + i * 6);
            break;
        }
    }
    return propValue;
}

bool Game::objectHasProperty(std::uint32_t objRef, std::uint16_t propId) {
    if (!isType(objRef, idObject)) {
        throw PlayError("Tried to test property of non-object");
    }

    const std::uint16_t propCount = readShort(objRef + 1);
    for (std::uint16_t i = 0; i < propCount; ++i) {
        std::uint16_t idHere = readShort(objRef + 3 + i * 6);
        if (idHere == propId) {
            return true;
        }
    }
    return false;
}

std::uint32_t Game::hasFlag(std::uint32_t address, std::uint32_t flags) const {
    return 0; // TODO
}

std::string Game::getNameOf(std::uint32_t address) {
    std::stringstream ss;

    int type = getType(address);
    switch(type) {
        case idString:
            return getString(address);
        case idObject: {
            std::uint32_t name = getObjectProperty(address, propName);
            if (name == 0) {
                ss << "[ObjectDef@" << std::hex << std::uppercase << address << "]";
            } else {
                std::uint32_t article = getObjectProperty(address, propArticle);
                if (article) {
                    ss << getString(article);
                }
                ss << getString(name);
            }
            return ss.str(); }
        default: {
            std::stringstream ss;
            ss << "[object#";
            ss << std::hex << std::uppercase << address;
            ss << '/' << (int) type << ']';
            return ss.str();
        }
    }
}

std::string Game::getPronoun(std::uint32_t cRef, int pronounType) {
    if (getObjectProperty(cRef, propClass) != ocCharacter) {
        throw PlayError("Tried to get pronoun for non-character");
    }
    Character *cDef = getCharacter(cRef);
    return getString(getObjectProperty(cDef->sex, pronounType));
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
        const char *aName = getString(getObjectProperty(a.itemIdent, propName));
        const char *bName = getString(getObjectProperty(b.itemIdent, propName));
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

Game::Character* Game::getCharacter(std::uint32_t address) {
    if (address < party.size()) {
        address = party[address];
    }

    auto theChar = characters.find(address);
    if (theChar != characters.end()) {
        return theChar->second;
    }

    resetCharacter(address);
    theChar = characters.find(address);
    if (theChar != characters.end()) {
        return theChar->second;
    }
    return nullptr;
}

const Game::Character* Game::getCharacter(std::uint32_t address) const {
    if (address < party.size()) {
        address = party[address];
    }

    auto theChar = characters.find(address);
    if (theChar != characters.end()) {
        return theChar->second;
    }
    return nullptr;
}

void Game::doRest(int forTime) {
    if (forTime <= 0) return;

    for (std::uint32_t whoRef : party) {
        for (unsigned skillNumber = 0; skillNumber < sklCount; ++skillNumber) {
            if (!testSkillFlags(skillNumber, sklVariable)) continue;
            int rate = skillRecoveryRate(skillNumber);
            if (rate == 0) continue;

            int result = rate * forTime / 100;
            if (result % 10 >= 5) {
                result = result / 10 + 1;
            } else {
                result = result / 10;
            }

            adjSkillCur(whoRef, skillNumber, result);

            if (whoRef == party[0]) {
                if (rate > 0) {
                    say("Gained ");
                    say(result);
                } else {
                    say("Lost ");
                    say(-result);
                }
                say(" ");
                say(getNameOf(readWord(readWord(headerSkillTable) + skillNumber*sklSize + sklName)));
                say(". ");
            }
        }
    }
    return;
}

bool Game::isKOed(std::uint32_t cRef) {
    for (unsigned i = 0; i < sklCount; ++i) {
        if (!testSkillFlags(i, sklVariable)) {
            continue;
        }
        if (testSkillFlags(i, sklKOFull) && getSkillCur(cRef, i) == getSkillMax(cRef, i)) {
            return true;
        }
        if (testSkillFlags(i, sklKOZero) && getSkillCur(cRef, i) == 0) {
            return true;
        }
    }
    return false;
}

int Game::skillRecoveryRate(int skillNo) {
    return readWord(readWord(headerSkillTable) + skillNo * sklSize + sklRecovery);
}

void Game::resetCharacter(std::uint32_t cRef) {
    auto oldChar = characters.find(cRef);
    if (oldChar != characters.end()) {
        delete oldChar->second;
        characters.erase(cRef);
    }

    Character *c = new Character;
    c->def = cRef;
    c->sex = getObjectProperty(cRef, propSex);
    c->species = getObjectProperty(cRef, propSpecies);
    characters.insert(std::make_pair(cRef, c));

    std::uint32_t skillSet = getObjectProperty(c->def, propSkills);
    for (int i = 0; i < sklCount; ++i) {
        c->skillAdj[i] = 0;
        c->skillCur[i] = 0;

        if (skillSet != 0) {
            if (testSkillFlags(i, sklVariable)) {
                if (testSkillFlags(i, sklKOFull)) {
                    c->skillCur[i] = 0;
                } else {
                    c->skillCur[i] = readShort(skillSet + 1 + i * 2);
                    if (testSkillFlags(i, sklX5)) {
                        c->skillCur[i] *= sklX5Multiplier;
                    }
                }
            }
        }
    }

    std::uint32_t gearList = getObjectProperty(cRef, propGear);
    if (gearList) {
        int count = readByte(gearList+1);
        for (int i = 0; i < count; ++i) {
            std::uint32_t itemRef = readWord(gearList+2+i*4);
            std::uint32_t slot = getObjectProperty(itemRef, propSlot);
            c->gear.insert(std::make_pair(slot, itemRef));
            std::uint32_t onEquip = getObjectProperty(itemRef, propOnEquip);
            if (onEquip) {
                doNode(onEquip);
            }
        }
    }
}

void Game::restoreCharacter(std::uint32_t cRef) {
    getCharacter(cRef);
    for (int i = 0; i < sklCount; ++i) {
        if (!testSkillFlags(i, sklVariable)) continue;
        if (testSkillFlags(i, sklKOFull)) {
            adjSkillCur(cRef, i, -getSkillMax(cRef, i));
        }
        if (testSkillFlags(i, sklKOZero)) {
            adjSkillCur(cRef, i, getSkillMax(cRef, i));
        }
    }
}

void Game::doDamage(std::uint32_t cRef, int amount, int to, int type) {
    adjSkillCur(cRef, to, -amount);
}

int Game::doSkillCheck(std::uint32_t cRef, int skill, int modifiers, int target) {
    std::uint32_t baseSkill = readByte(readWord(headerSkillTable)+sklSize*skill+sklBaseSkill);
    int dieResult = roll(3,6);
    int baseSkillLvl = getSkillCur(cRef, baseSkill);
    int skillLvl = getSkillCur(cRef, skill);
    int finalLvl = skillLvl + baseSkillLvl;

    say("[");
    if (baseSkill) {
        say(baseSkillLvl);
        say("+");
        say(skillLvl);
        say("=");
        say(finalLvl);
    } else {
        say(skillLvl);
    }
    if (modifiers) {
        say(" + ");
        say(modifiers);
    }
    int result = finalLvl + modifiers + dieResult - target;

    say(" + ");
    say(dieResult);
    say("(3d6) vs ");
    say(target);
    say(" = ");
    say(result);
    say("] ");
    return result;
}

bool Game::testSkillFlags(int skillNo, uint32_t flags) {
    std::uint32_t theFlags = readByte(readWord(headerSkillTable)+sklSize*skillNo+sklFlags);
    return (flags & theFlags) == flags;
}

int Game::getSkillMax(std::uint32_t cRef, int skillNo) {
    Character *c = getCharacter(cRef);
    if (!c) return 0;

    std::uint32_t skillSet = getObjectProperty(c->def, propSkills);
    int base = 0;
    if (skillSet != 0) {
        base = readShort(skillSet + 1 + skillNo * 2);
    }

    for (auto item : c->gear) {
        std::uint32_t itemSkills = getObjectProperty(item.second, propSkills);
        if (itemSkills) {
            base += static_cast<short>(readShort(itemSkills + 1 + skillNo * 2));
        }
    }

    base += c->skillAdj[skillNo];

    if (testSkillFlags(skillNo, sklX5)) {
        base *= sklX5Multiplier;
    }
    return base;
}

void Game::adjSkillMax(std::uint32_t cRef, int skillNo, int adjustment) {
    Character *c = getCharacter(cRef);
    if (!c) return;

    c->skillAdj[skillNo] += adjustment;
}

int Game::getSkillCur(std::uint32_t cRef, int skillNo) {
    Character *c = getCharacter(cRef);
    if (testSkillFlags(skillNo, sklVariable)) {
        if (!c) return 0;
        return c->skillCur[skillNo];
    } else {
        return getSkillMax(cRef, skillNo);
    }
}

void Game::adjSkillCur(std::uint32_t cRef, int skillNo, int adjustment) {
    Character *c = getCharacter(cRef);
    if (!c) return;

    int cur = c->skillCur[skillNo];
    int max = getSkillMax(cRef, skillNo);

    cur += adjustment;
    if (cur < 0)    cur = 0;
    if (cur > max)  cur = max;

    c->skillCur[skillNo] = cur;
}

std::vector<std::uint32_t> Game::getActions(std::uint32_t cRef) {
    std::uint32_t list;
    std::vector<std::uint32_t> actions;
    Character *c = getCharacter(cRef);
    if (!c) return actions;

    const std::uint32_t weaponSlot = readWord(headerWeaponSlot);
    if (c->gear.count(weaponSlot) == 0) {
        list = getObjectProperty(cRef, propBaseAbilities);
        if (list) {
            unsigned count = readByte(list+1);
            for (unsigned int i = 0; i < count; ++i) {
                actions.push_back(readWord(list+2+i*4));
            }
        }
    }

    list = getObjectProperty(cRef, propExtraAbilities);
    if (list) {
        unsigned count = readByte(list+1);
        for (unsigned int i = 0; i < count; ++i) {
            actions.push_back(readWord(list+2+i*4));
        }
    }

    for (const auto &item : c->gear) {
        list = getObjectProperty(item.second, propActionList);
        if (list) {
            unsigned count = readByte(list+1);
            for (unsigned int i = 0; i < count; ++i) {
                actions.push_back(readWord(list+2+i*4));
            }
        }
    }

    return actions;
}

void Game::doGameSetup() {
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

    for (unsigned i = 0; i < storageTempCount; ++i) {
        storage.erase(storageFirstTemp-i);
    }

    if (gameStarted && (!inLocation || newLocation) && !inCombat) {
        gameTime += 2;
    }

    if (startedCombat) {
        startedCombat = false;
        random_shuffle(combatants.begin(), combatants.end());
        say("\n");
        doCombatLoop();
    }
}

void Game::doCombatLoop() {
    while (getObjectProperty(combatants[currentCombatant], propFaction) != 0 ||
            isKOed(combatants[currentCombatant])) {
        if (!isKOed(combatants[currentCombatant])) {
            std::uint32_t ai = getObjectProperty(combatants[currentCombatant], propAi);
            if (ai > 0) {
                setTemp(0, combatants[currentCombatant]);
                newNode(ai);
            } else {
                say(toUpperFirst(getNameOf(combatants[currentCombatant])));
                say(" takes a turn.\n");
            }
        }
        int status = combatStatus();
        if (status != 0) {
            inCombat = false;
            say("Combat is over.\n");
            options.push_back(Option(1, afterCombatNode));
            return;
        }
        advanceCombatant();
    }

    auto actionList = getActions(combatants[currentCombatant]);
    for (auto action : actionList) {
        std::uint32_t node = getObjectProperty(action, propCombatNode);
        if (node) {
            options.push_back(Option(getObjectProperty(action, propName), node));
        }
    }
    options.push_back(Option(readWord(headerWeaponSlot), 0));

    say("What does ");
    say(getNameOf(combatants[currentCombatant]));
    say(" do?\n");
}

int Game::combatStatus() {
    int allies = 0, enemies = 0;
    for (std::uint32_t whoRef : combatants) {
        if (isKOed(whoRef)) continue;
        if (getObjectProperty(whoRef, propFaction) == 0) {
            ++allies;
        } else {
            ++enemies;
        }
    }

    if (allies == 0)    return -1;
    if (enemies == 0)   return 1;
    return 0;
}

void Game::advanceCombatant() {
    ++currentCombatant;
    if (currentCombatant >= combatants.size()) {
        say("Combat round ");
        say(++combatRound);
        say(" begins.\n");
        currentCombatant = 0;
        gameTime += 1;
    }
}

void Game::doOption(int optionNumber) {
    if (optionNumber < 0 || optionNumber >= (signed)options.size()) {
        return;
    }

    std::uint32_t dest = options[optionNumber].dest;
    uint32_t nameAddr = options[optionNumber].name;
    clearOutput();

    if (inCombat) {
        options.clear();
        if (dest == 0) {
            say(toUpperFirst(getNameOf(combatants[currentCombatant])));
            say(" does nothing.\n");
        } else {
            setTemp(0, combatants[currentCombatant]);
            newNode(dest);
        }
        advanceCombatant();
        doCombatLoop();

    } else {
        if (dest == 0) {
            dest = location;
        }

        say("\n> ");
        if (nameAddr == 1) {
            say("Continue");
        } else {
            say(getString(nameAddr));
        }
        say("\n\n");

        if (options[optionNumber].extra) {
            push(options[optionNumber].extra);
        }
        newNode(dest);
    }
}

void Game::useItem(int itemNumber) {
    if (itemNumber < 0 || itemNumber >= (signed)inventory.size()) {
        return;
    }

    uint32_t item = inventory[itemNumber].itemIdent;
    if (!item) return;

    uint32_t onUse = getObjectProperty(item, propOnUse);
    if (!onUse) return;

    uint32_t article = getObjectProperty(item, propArticle);
    uint32_t name = getObjectProperty(item, propName);

    clearOutput();
    say("\n> Using ");
    say(getString(article));
    say(getString(name));
    say("\n\n");

    newNode(onUse);
}

void Game::equipItem(std::uint32_t whoIdent, int itemNumber) {
    if (itemNumber < 0 || itemNumber >= (signed)inventory.size()) {
        return;
    }

    Character *who = getCharacter(whoIdent);
    if (!who) {
        return;
    }

    uint32_t item = inventory[itemNumber].itemIdent;
    if (!item) return;

    uint32_t slot = getObjectProperty(item, propSlot);
    if (!slot) return;

    if (who->gear.count(slot) > 0) {
        std::uint32_t oldItem = who->gear[slot];
        std::uint32_t onRemove = getObjectProperty(oldItem, propOnRemove);
        if (onRemove) {
            doNode(onRemove);
        }
        addItems(1, oldItem);
        who->gear.erase(slot);
    }

    removeItems(1, item);
    std::uint32_t onEquip = getObjectProperty(item, propOnEquip);
    if (onEquip) {
        doNode(onEquip);
    }
    who->gear.insert(std::make_pair(slot, item));
}

void Game::unequipItem(std::uint32_t whoIdent, std::uint32_t slotIdent) {
    Character *who = getCharacter(whoIdent);
    if (!who) {
        return;
    }

    auto gearIter = who->gear.find(slotIdent);
    if (gearIter == who->gear.end()) {
        return;
    }

    addItems(1, gearIter->second);
    who->gear.erase(slotIdent);
}

void Game::doAction(std::uint32_t cRef, std::uint32_t action) {
    if (cRef == 0 || action == 0) {
        return;
    }
    uint32_t peaceNode = getObjectProperty(action, propPeaceNode);
    if (!peaceNode) {
        return;
    }

    clearOutput();
    say("\n> ");
    say(toUpperFirst(getNameOf(cRef)));
    say (" uses their ");
    say(getNameOf(action));
    say(" ability\n\n");

    newNode(peaceNode);
}

bool Game::actionAllowed() const {
    return inLocation && isRunning;
}

bool Game::isInCombat() const {
    return inCombat;
}

void Game::say(const std::string &text) {
    if (text.empty()) return;
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

void Game::setTemp(unsigned tempNo, std::uint32_t value) {
    if (tempNo >= storageTempCount) {
        throw PlayError("Tried to update bad temp storage position");
    }
    storage[storageFirstTemp - tempNo] = value;
}

void Game::addList(std::uint32_t ident, std::shared_ptr<List> list) {
    dynamicLists.insert(std::make_pair(ident, std::shared_ptr<List>(list)));
}

std::shared_ptr<Game::List> Game::getList(std::uint32_t ident) {
    auto i = dynamicLists.find(ident);
    if (i != dynamicLists.end()) {
        return i->second;
    }
    return nullptr;
}

void Game::freeList(std::uint32_t ident) {
    auto i = dynamicLists.find(ident);
    if (i != dynamicLists.end()) {
        dynamicLists.erase(i);
    }
}