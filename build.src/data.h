#ifndef DATA_H
#define DATA_H

#include <memory>
#include <string>
#include <list>


class Value {
public:
    Value()
    : type(Integer), value(0)
    { }
    Value(const std::string &text)
    : type(Identifier), text(text), value(0)
    { }
    Value(int value)
    : type(Integer), value(value)
    { }
    enum Type {
        Identifier, Integer
    };

    Type type;
    std::string text;
    int value;
};

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

    std::string article, singular, plural;
    Value onUse;
};
std::ostream& operator<<(std::ostream &out, const Value &type);

#endif
