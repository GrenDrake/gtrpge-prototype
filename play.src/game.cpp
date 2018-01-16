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
    nextDataItem = size + 32;

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
    return outputBuffer;
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

std::string Game::getNameOf(std::uint32_t address) {
    std::stringstream ss;

    if (address > dataSize) {
        switch(dataType(address)) {
            case RuntimeData::NoneType:
                ss << "[unused address#";
                ss << std::hex << std::setw(8) << std::setfill('0');
                ss << address << "]";
                return ss.str();
            default:
                ss << std::hex << std::setw(8) << std::setfill('0');
                ss << "[high memory object#"<<address<<"]";
                return ss.str();
        }
    }

    int type = getType(address);
    int work;
    switch(type) {
        case idString:
            return getString(address);
        case idCharacter: {
            Character *c = getCharacter(address);
            ss << getString(readWord(c->def+chrArticle));
            ss << getString(readWord(c->def+chrName));
            return ss.str(); }
        case idItem:
            work = address + itmArticle;
            ss << getString(readWord(work));
            work = address + itmSingular;
            ss << getString(readWord(work));
            return ss.str();
        case idSex:
            work = address + sexName;
            return getString(readWord(work));
        case idSpecies:
            work = address + spcName;
            return getString(readWord(work));
        case idAction:
            work = address + actName;
            return getString(readWord(work));
        default: {
            std::stringstream ss;
            ss << "[object#";
            ss << std::hex << std::uppercase << address;
            ss << '/' << (int) type << ']';
            return ss.str();
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

Game::Character* Game::getCharacter(std::uint32_t address) {
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

void Game::resetCharacter(std::uint32_t cRef) {
    auto oldChar = characters.find(cRef);
    if (oldChar != characters.end()) {
        delete oldChar->second;
        characters.erase(cRef);
    }

    Character *c = new Character;
    c->def = cRef;
    c->sex = getProperty(cRef, chrSex);
    c->species = getProperty(cRef, chrSpecies);
    characters.insert(std::make_pair(cRef, c));

    std::uint32_t skillSet = getProperty(c->def, chrSkillDefaults);
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

    std::uint32_t gearList = getProperty(cRef, chrGearList);
    if (gearList) {
        int count = readByte(gearList+1);
        for (int i = 0; i < count; ++i) {
            std::uint32_t itemRef = readWord(gearList+2+i*4);
            std::uint32_t slot = getProperty(itemRef, itmSlot);
            c->gear.insert(std::make_pair(slot, itemRef));
            std::uint32_t onEquip = getProperty(itemRef, itmOnEquip);
            if (onEquip) {
                doNode(onEquip);
            }
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
    say(" + ");
    say(dieResult);
    say("(3d6) vs ");
    say(target);
    say("]");
    return (finalLvl + modifiers + dieResult) - target;
}

bool Game::testSkillFlags(int skillNo, uint32_t flags) {
    std::uint32_t theFlags = readByte(readWord(headerSkillTable)+sklSize*skillNo+sklFlags);
    return (flags & theFlags) == flags;
}

int Game::getSkillMax(std::uint32_t cRef, int skillNo) {
    Character *c = getCharacter(cRef);
    if (!c) return 0;

    std::uint32_t skillSet = getProperty(c->def, chrSkillDefaults);
    int base = 0;
    if (skillSet != 0) {
        base = readShort(skillSet + 1 + skillNo * 2);
    }

    for (auto item : c->gear) {
        std::uint32_t itemSkills = getProperty(item.second, itmSkillSet);
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
        list = getProperty(cRef, chrBaseAbilities);
        if (list) {
            unsigned count = readByte(list+1);
            for (unsigned int i = 0; i < count; ++i) {
                actions.push_back(readWord(list+2+i*4));
            }
        }
    }

    list = getProperty(cRef, chrExtraAbilities);
    if (list) {
        unsigned count = readByte(list+1);
        for (unsigned int i = 0; i < count; ++i) {
            actions.push_back(readWord(list+2+i*4));
        }
    }

    for (const auto &item : c->gear) {
        list = getProperty(item.second, itmActionList);
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

    if (gameStarted && (!inLocation || newLocation)) {
        gameTime += 2;
    }
}

void Game::doOption(int optionNumber) {
    if (optionNumber < 0 || optionNumber >= (signed)options.size()) {
        return;
    }

    std::uint32_t dest = options[optionNumber].dest;
    if (dest == 0) {
        dest = location;
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

    if (options[optionNumber].extra) {
        push(options[optionNumber].extra);
    }
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

    uint32_t slot = getProperty(item, itmSlot);
    if (!slot) return;

    if (who->gear.count(slot) > 0) {
        std::uint32_t oldItem = who->gear[slot];
        std::uint32_t onRemove = getProperty(oldItem, itmOnRemove);
        if (onRemove) {
            doNode(onRemove);
        }
        addItems(1, oldItem);
        who->gear.erase(slot);
    }

    removeItems(1, item);
    std::uint32_t onEquip = getProperty(item, itmOnEquip);
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
    uint32_t peaceNode = getProperty(action, actPeaceNode);
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

void Game::say(const std::string &text) {
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

void Game::freeData(std::uint32_t ident) {
    auto i = runtimeData.find(ident);
    if (i != runtimeData.end()) {
        runtimeData.erase(i);
    }
}