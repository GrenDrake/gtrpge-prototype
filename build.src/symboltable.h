#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <string>
#include <vector>
#include "origin.h"

class SymbolDef {
public:
    enum Type {
        Node, Constant, Item, Sex, Species, Skill, Character, DamageType, ObjectDef, Integer, None, String
    };

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
private:
    std::vector<SymbolDef> symbols;
};


#endif