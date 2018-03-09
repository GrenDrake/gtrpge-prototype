#include <array>
#include <iomanip>
#include <iostream>
#include <ostream>

#include "build.h"
#include "symboltable.h"

void writeByte(std::ostream &out, std::uint8_t value);
void writeShort(std::ostream &out, std::uint16_t value);
void writeWord(std::ostream &out, std::uint32_t value);

void writeValue(std::ostream &out, const Origin &origin, const Value &value);
void writeFlags(std::ostream &out, const Origin &origin, const std::vector<Value> &flags);
void writeLabelValue(std::ostream &out, const std::string &labelName);

uint32_t processValue(const Origin &origin, const Value &value, const std::string &nodeName);

/* ************************************************************************
 * OBJECT DEF STUFF                                                       */

std::unordered_map<std::string, std::uint16_t> ObjectDef::propertyNames;
std::uint16_t ObjectDef::nextProperty = propFirstCustom;

std::uint32_t ObjectDef::nextIdent = 1;

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
    if (type == FlagSet) {
        if (mFlagSet.size() != rhs.mFlagSet.size()) {
            return false;
        }
        for (const auto &outer : mFlagSet) {
            for (const auto &inner : mFlagSet) {
                if (inner == outer) {
                    continue;
                }
            }
            return false;
        }
        return true;
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


void DataList::write(std::ostream &out, const SymbolTable &symbols) {
    writeByte(out, idList);
    writeByte(out, values.size());
    for (const Value &value : values) {
        writeValue(out, origin, value);
    }
}


void SkillSet::write(std::ostream &out, const SymbolTable &symbols) {
    writeByte(out, idSkillSet);
    writeWord(out, skillMap.size());

    for (auto iter : skillMap) {
        writeValue(out, origin, iter.first);
        writeValue(out, origin, iter.second);
    }
}

void ObjectDef::write(std::ostream &out, const SymbolTable &symbols) {
    writeByte(out, idObject);
    writeShort(out, properties.size());

    for (auto iter = properties.begin(); iter != properties.end(); ++iter) {
    // for (auto prop : properties) {
        auto prop = *iter;
        std::uint32_t nextPos = 0;
        if (std::next(iter) != properties.end()) {
            nextPos = out.tellp();
            nextPos += objPropSize;
        }

        //  prop-id  type-id  padd  next  value
        //  0/2      2/1      3/1   4/4   8/4
        writeShort(out, prop.first);
        writeByte(out, 0); // type ID
        writeByte(out, 0); // padding
        writeValue(out, origin, prop.second);
    }
}

