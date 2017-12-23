#ifndef DATA_H
#define DATA_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <list>

#include "../play.src/constants.h"

class Value {
public:
    enum Type {
        Identifier, Integer
    };

    Value()
    : type(Integer), value(0)
    { }
    Value(const std::string &text)
    : type(Identifier), text(text), value(0)
    { }
    Value(int value)
    : type(Integer), value(value)
    { }

    bool operator==(const Value &rhs) const {
        if (rhs.type != type) {
            return false;
        }
        if (type == Integer && rhs.value != value) {
            return false;
        }
        return rhs.text == text;
    }

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
    std::list<Value> parts;
    std::uint32_t pos;
};

class Block {
public:
    std::list<std::shared_ptr<Statement> > statements;
};

class DataType {
public:
    virtual ~DataType() {

    }
    virtual size_t getSize() const = 0;

    Origin origin;
    std::string name;
    std::uint32_t pos;
};

class Node {
public:
    std::string name;
    std::shared_ptr<Block> block;
};

class SpeciesDef {
public:
    Origin origin;
    std::string name;

    std::string displayName;
    std::unordered_set<Value> flags;
};

class SexDef : public DataType {
public:
    virtual size_t getSize() const {
        return sexSize;
    }

    std::string displayName;
    std::unordered_set<Value> flags;
    std::string subject, object, possess, adject, reflex;
};

class SkillDef {
public:
    Origin origin;
    std::string name;

    Value statSkill;
    std::string displayName;
    Value defaultValue;
};

class CharacterDef {
public:
    Origin origin;
    std::string name;

    std::string article, displayName;
    Value sex, species;
    std::unordered_set<Value> flags;
    Value faction;
    std::unordered_map<std::string, Value> skills;
    std::unordered_map<std::string, std::string> gear;
};

class ItemDef : public DataType {
public:
    virtual size_t getSize() const {
        return itmSize;
    }

    std::unordered_set<Value> flags;
    std::string article, singular, plural;
    Value onUse;
};
std::ostream& operator<<(std::ostream &out, const Value &type);

#endif
