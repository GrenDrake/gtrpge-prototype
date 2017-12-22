#ifndef DATA_H
#define DATA_H

#include <memory>
#include <unordered_set>
#include <string>
#include <list>


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

class Node {
public:
    std::string name;
    std::shared_ptr<Block> block;
};

class ItemDef {
public:
    Origin origin;
    std::string name;

    std::unordered_set<Value> flags;
    std::string article, singular, plural;
    Value onUse;
};
std::ostream& operator<<(std::ostream &out, const Value &type);

#endif
