#include <ostream>

#include "build.h"

void writeByte(std::ostream &out, std::uint8_t value);
void writeShort(std::ostream &out, std::uint16_t value);
void writeWord(std::ostream &out, std::uint32_t value);

void writeValue(std::ostream &out, const Origin &origin, const Value &value);
void writeFlags(std::ostream &out, const Origin &origin, const std::unordered_set<Value> &flags);
void writeLabelValue(std::ostream &out, const std::string &labelName);

bool Value::operator==(const Value &rhs) const {
    if (rhs.type != type) {
        return false;
    }
    if (type == Integer && rhs.value != value) {
        return false;
    }
    return rhs.text == text;
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
    writeWord(out, skills.size());
    writeWord(out, gear.size());
    for (const auto &skill : skills) {
        writeLabelValue(out, skill.first);
        writeValue(out, origin, skill.second);
    }
    for (const auto &gearItem : gear) {
        writeLabelValue(out, gearItem.first);
        writeLabelValue(out, gearItem.second);
    }
}

void ItemDef::write(std::ostream &out) {
    writeByte(out, idItem);
    writeFlags(out, origin, flags);
    writeLabelValue(out, article);
    writeLabelValue(out, singular);
    writeLabelValue(out, plural);
    writeValue(out, origin, onUse);
}
