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



void SpeciesDef::write(std::ostream &out) {
    writeByte(out, idSpecies);
    writeFlags(out, origin, flags);
    writeLabelValue(out, displayName);
}


void SexDef::write(std::ostream &out) {
    writeByte(out, idSex);
    writeFlags(out, origin, flags);
    writeLabelValue(out, displayName);
    writeLabelValue(out, subject);
    writeLabelValue(out, object);
    writeLabelValue(out, possess);
    writeLabelValue(out, adject);
    writeLabelValue(out, reflex);
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
