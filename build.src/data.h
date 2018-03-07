#ifndef DATA_H
#define DATA_H

#include <array>
#include <functional>
#include <memory>
#include <unordered_map>
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
        Identifier, Global, Integer, FlagSet
    };

    Value()
    : type(Integer), value(0)
    { }
    explicit Value(const std::string &text)
    : type(Identifier), text(text), value(0)
    { }
    Value(Type type, const std::string &text)
    : type(type), text(text), value(0)
    { }
    explicit Value(int value)
    : type(Integer), value(value)
    { }
    explicit Value(Type type)
    : type(type), value(0)
    { }

    bool operator==(const Value &rhs) const;

    Type type;
    std::string text;
    int value;
    std::vector<Value> mFlagSet;
};

namespace std {
    template <>
    struct hash<Value> {
        std::size_t operator()(const Value& k) const {
            std::size_t result = 0;
            switch(k.type) {
                case Value::Identifier:
                case Value::Global: {
                    std::hash<std::string> stringHash;
                    return stringHash(k.text); }
                case Value::Integer:
                    return k.value;
                case Value::FlagSet: {
                    std::hash<Value> valueHash;
                    for (const Value &outer : k.mFlagSet) {
                        result ^= valueHash(outer);
                    }
                    return result; }
                default:
                    return 0;
            }
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
    int recoveryRate; // points recovered (or lost if negative) per 1,000th of a minute
    std::vector<Value> flags;
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

    static std::uint32_t nextIdent;

    virtual size_t getSize() const {
        // idObject + (properties * 6)
        return 1 + properties.size() * objPropSize;
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
    { }
    virtual size_t getSize() const {
        return 5 + 8 * skillMap.size();
    }
    virtual void write(std::ostream &out);
    virtual std::string getTypeName() const {
        return "MAP";
    }

    std::unordered_map<Value, Value> skillMap;
    // std::array<std::uint16_t, sklCount> skills;
    // bool setDefaults;
};

std::ostream& operator<<(std::ostream &out, const Value &type);
std::ostream& operator<<(std::ostream &out, const DataType &data);

#endif
