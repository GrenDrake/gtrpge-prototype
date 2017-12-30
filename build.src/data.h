#ifndef DATA_H
#define DATA_H

#include <array>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <list>
#include <vector>

#include "../play.src/constants.h"

class Command {
public:
    const char *text;
    int code;
    int argCount;
};

class Value {
public:
    enum Type {
        Identifier, Global, Integer
    };

    Value()
    : type(Integer), value(0)
    { }
    Value(const std::string &text)
    : type(Identifier), text(text), value(0)
    { }
    Value(Type type, const std::string &text)
    : type(type), text(text), value(0)
    { }
    Value(int value)
    : type(Integer), value(value)
    { }

    bool operator==(const Value &rhs) const;

    Type type;
    std::string text;
    int value;
};
namespace std {
    template<>
    struct hash<Value> {
        typedef Value argument_type;
        typedef size_t result_type;

        size_t operator()(const Value &x) const {
            if (x.type == Value::Integer) {
                return x.value;
            }
            return hash<std::string>{}(x.text);
        }
    };
}

class Statement {
public:
    Origin origin;
    std::vector<Value> parts;
    std::uint32_t pos;

    const Command *commandInfo;
};

class Block {
public:
    std::vector<std::shared_ptr<Statement> > statements;
};

class DataType {
public:
    virtual ~DataType() {

    }
    virtual size_t getSize() const = 0;
    virtual void write(std::ostream &out) = 0;

    Origin origin;
    std::string name;
    std::uint32_t pos;
};

class Node {
public:
    Origin origin;
    std::string name;
    std::shared_ptr<Block> block;
};

class SpeciesDef : public DataType {
public:
    virtual size_t getSize() const {
        return spcSize;
    }
    virtual void write(std::ostream &out);

    std::string displayName;
    std::unordered_set<Value> flags;
};

class SexDef : public DataType {
public:
    virtual size_t getSize() const {
        return sexSize;
    }
    virtual void write(std::ostream &out);

    std::string displayName;
    std::unordered_set<Value> flags;
    std::string subject, object, possess, adject, reflex;
};

class SkillDef {
public:
    virtual size_t getSize() const {
        return sklSize;
    }
    virtual void write(std::ostream &out) {
    }

    Origin origin;
    std::string name;
    Value statSkill;
    std::string displayName;
    int defaultValue;
    std::unordered_set<Value> flags;
};

class CharacterDef : public DataType {
public:
    virtual size_t getSize() const {
        return chrGearList + gear.size()*cgSize + 4;
    }
    virtual void write(std::ostream &out);

    std::string article, displayName;
    Value sex, species;
    std::unordered_set<Value> flags;
    Value faction;
    std::unordered_map<std::string, Value> skillMap;
    std::array<std::uint8_t, sklCount> skills;
    std::unordered_map<std::string, std::string> gear;
};

class ItemDef : public DataType {
public:
    virtual size_t getSize() const {
        return itmSize;
    }
    virtual void write(std::ostream &out);

    std::unordered_set<Value> flags;
    std::string article, singular, plural;
    Value onUse;
};
std::ostream& operator<<(std::ostream &out, const Value &type);

#endif
