#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <iosfwd>
#include <string>
#include <vector>
#include "origin.h"

class SymbolDef {
public:
    enum Type {
        Node, Constant, Map, List, Skill, DamageType, ObjectDef, Unknown, String
    };

    static int toId(Type);
    static const char* typeName(Type type);

    SymbolDef(const Origin &origin, const std::string &name, Type type)
    : origin(origin), name(name), type(type)
    { }

    Origin origin;
    std::string name;
    Type type;
};

class SymbolTable {
public:
    void add(const Origin &origin, const std::string &name, SymbolDef::Type);
    const SymbolDef* get(const std::string &name) const;
    bool exists(const std::string &name) const;
    SymbolDef::Type type(const std::string &name) const;
    void dump(std::ostream &out) const;
private:
    std::vector<SymbolDef> symbols;
};


#endif