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

class DataType {
public:
    virtual ~DataType() {

    }
    virtual size_t getSize() const = 0;
    virtual void write(std::ostream &out) = 0;
    virtual std::string getTypeName() const = 0;
    void prettyPrint(std::ostream &out) const;

    Origin origin;
    std::string name;
    std::uint32_t pos;
};

class ObjectDef : public DataType {
public:
    static std::unordered_map<std::string, std::uint16_t> propertyNames;
    static std::uint16_t nextProperty;
    static void setPropertyIdent(const std::string &name, std::uint16_t id);
    static std::uint16_t getPropertyIdent(const std::string &propertyName);

    virtual size_t getSize() const {
        // idObject + propCount + (properties * 6)
        return 3 + properties.size() * 6;
    }
    virtual void write(std::ostream &out);
    virtual std::string getTypeName() const {
        return "OBJECT";
    }

    bool hasProperty(const std::string &propName);
    bool hasProperty(std::uint16_t propId);

    std::uint32_t ident;
    std::unordered_map<std::uint16_t, Value> properties;
};

class Node {
public:
    Origin origin;
    std::string name;
    std::shared_ptr<Block> block;
};

class SexDef : public DataType {
public:
    virtual size_t getSize() const {
        return sexSize;
    }
    virtual void write(std::ostream &out);
    virtual std::string getTypeName() const {
        return "SEX";
    }

    std::string displayName;
    std::unordered_set<Value> flags;
    std::string subject, object, possess, adject, reflex;
};

class CharacterDef : public DataType {
public:
    virtual size_t getSize() const {
        return chrSize;
    }
    virtual void write(std::ostream &out);
    virtual std::string getTypeName() const {
        return "CHARACTER";
    }

    std::string article, displayName;
    Value sex, species;
    std::unordered_set<Value> flags;
    Value faction;
    std::string baseAbilities, extraAbilities;
    std::string gearList, skillSet;
};

class ItemDef : public DataType {
public:
    virtual size_t getSize() const {
        return itmSize;
    }
    virtual void write(std::ostream &out);
    virtual std::string getTypeName() const {
        return "ITEM";
    }

    std::unordered_set<Value> flags;
    std::string article, singular, plural, description;
    Value onUse, canEquip, onEquip, onRemove, slot;
    std::string actionsList, skillSet;
};

class DataList : public DataType {
public:
    virtual size_t getSize() const {
        return 2 + values.size() * 4;
    }
    virtual void write(std::ostream &out);
    virtual std::string getTypeName() const {
        return "LIST";
    }

    std::vector<Value> values;
};

class SkillSet : public DataType {
public:
    SkillSet()
    : setDefaults(false)
    { }
    virtual size_t getSize() const {
        return sklSetSize;
    }
    virtual void write(std::ostream &out);
    virtual std::string getTypeName() const {
        return "SKILLSET";
    }

    std::unordered_map<std::string, Value> skillMap;
    std::array<std::uint16_t, sklCount> skills;
    bool setDefaults;
};

class ActionDef : public DataType {
public:
    virtual size_t getSize() const {
        return actSize;
    }
    virtual void write(std::ostream &out);
    virtual std::string getTypeName() const {
        return "ACTION";
    }

    std::string energyStat;
    std::uint32_t energyCost;
    std::string displayName;
    Value combatNode, peaceNode;
};

std::ostream& operator<<(std::ostream &out, const Value &type);
std::ostream& operator<<(std::ostream &out, const DataType &data);

#endif
