#include <array>
#include <iomanip>
#include <ostream>

#include "build.h"

void writeByte(std::ostream &out, std::uint8_t value);
void writeShort(std::ostream &out, std::uint16_t value);
void writeWord(std::ostream &out, std::uint32_t value);

void writeValue(std::ostream &out, const Origin &origin, const Value &value);
void writeFlags(std::ostream &out, const Origin &origin, const std::unordered_set<Value> &flags);
void writeLabelValue(std::ostream &out, const std::string &labelName);

/* ************************************************************************
 * OBJECT DEF STUFF                                                       */

std::unordered_map<std::string, std::uint16_t> ObjectDef::propertyNames;
std::uint16_t ObjectDef::nextProperty = 200;

void ObjectDef::setPropertyIdent(const std::string &name, std::uint16_t id) {
    auto iter = propertyNames.find(name);
    if (iter != propertyNames.end()) {
        throw BuildError(Origin(), "Built-in property ID has name conflict");
    }
    propertyNames.insert(std::make_pair(name, id));
}

std::uint16_t ObjectDef::getPropertyIdent(const std::string &propertyName) {
    auto iter = propertyNames.find(propertyName);
    if (iter != propertyNames.end()) {
        return iter->second;
    }
    std::uint16_t myId = nextProperty++;
    propertyNames.insert(std::make_pair(propertyName, myId));
    return myId;
}

bool ObjectDef::hasProperty(const std::string &propName) {
    return hasProperty(getPropertyIdent(propName));
}

bool ObjectDef::hasProperty(std::uint16_t propId) {
    auto iter = properties.find(propId);
    return iter != properties.end();
}

/* ************************************************************************
 * OPERATOR OVERLOADS FOR DATA TYPES                                      */

bool Value::operator==(const Value &rhs) const {
    if (rhs.type != type) {
        return false;
    }
    if (type == Integer && rhs.value != value) {
        return false;
    }
    return rhs.text == text;
}

std::ostream& operator<<(std::ostream &out, const Value &type) {
    if (type.type == Value::Identifier) {
        out << type.text;
    } else {
        out << type.value;
    }
    return out;
}

std::ostream& operator<<(std::ostream &out, const DataType &data) {
    data.prettyPrint(out);
    return out;
}


/* ************************************************************************
 * DATA TYPE OUTPUT METHODS                                               */

void DataType::prettyPrint(std::ostream &out) const {
    out << std::left;
    out << std::setw(32) << name << "   ";
    out << std::setw(10) << getTypeName() << "   ";
    out << origin;
}



void CharacterDef::write(std::ostream &out) {
    writeByte(out, idCharacter);
    writeFlags(out, origin, flags);
    writeLabelValue(out, article);
    writeLabelValue(out, displayName);
    writeValue(out, origin, sex);
    writeValue(out, origin, species);
    writeValue(out, origin, faction);
    writeLabelValue(out, skillSet);
    writeLabelValue(out, gearList);
    writeLabelValue(out, baseAbilities);
    writeLabelValue(out, extraAbilities);
}


void ItemDef::write(std::ostream &out) {
    writeByte(out, idItem);
    writeFlags(out, origin, flags);
    writeLabelValue(out, article);
    writeLabelValue(out, singular);
    writeLabelValue(out, plural);
    if (description.empty()) {
        writeWord(out, 0);
    } else {
        writeLabelValue(out, description);
    }
    writeValue(out, origin, onUse);
    writeValue(out, origin, canEquip);
    writeValue(out, origin, onEquip);
    writeValue(out, origin, onRemove);
    writeValue(out, origin, slot);
    writeLabelValue(out, actionsList);
    writeLabelValue(out, skillSet);
}


void DataList::write(std::ostream &out) {
    writeByte(out, idList);
    writeByte(out, values.size());
    for (const Value &value : values) {
        writeValue(out, origin, value);
    }
}


void SkillSet::write(std::ostream &out) {
    writeByte(out, idSkillSet);

    for (std::uint16_t val : skills) {
        writeShort(out, val);
    }
}

void ActionDef::write(std::ostream &out) {
    writeByte(out, idAction);
    writeLabelValue(out, energyStat);
    writeWord(out, energyCost);
    writeLabelValue(out, displayName);
    writeValue(out, origin, combatNode);
    writeValue(out, origin, peaceNode);
}

void ObjectDef::write(std::ostream &out) {
    writeByte(out, idObject);
    writeShort(out, properties.size());
    for (auto prop : properties) {
        writeShort(out, prop.first);
        writeValue(out, origin, prop.second);
    }
}

