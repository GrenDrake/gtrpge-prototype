#include <iomanip>
#include <ostream>
#include <string>
#include <sstream>

#include "../play.src/constants.h"
#include "builderror.h"
#include "symboltable.h"

int SymbolDef::toId(Type type) {
    switch(type) {
        case Node:      return pidReference;
        case Constant:  return pidInteger;
        case Map:       return pidReference;
        case List:      return pidReference;
        case Skill:     return pidInteger;
        case DamageType:return pidInteger;
        case ObjectDef: return pidReference;
        case Unknown:   return pidInteger;
        case String:    return pidReference;
        default:        return pidUndefined;
    }
}
const char* SymbolDef::typeName(Type type) {
    switch(type) {
        case Node:      return "Node";
        case Constant:  return "Constant";
        case Map:       return "Map";
        case List:      return "List";
        case Skill:     return "Skill";
        case DamageType:return "DamageType";
        case ObjectDef: return "ObjectDef";
        case Unknown:   return "Unknown";
        case String:    return "String";
        default:        return "Unahndled Type Name";
    }
}

void SymbolTable::add(const Origin &origin, const std::string &name, SymbolDef::Type type) {
    for (const auto &i : symbols) {
        if (i.name == name) {
            std::stringstream ss;
            ss << "Symbol " << name << " was already defined at " << i.origin << ".";
            throw BuildError(origin, ss.str());
        }
    }
    symbols.push_back(SymbolDef(origin, name, type));
}

const SymbolDef* SymbolTable::get(const std::string &name) const {
    for (const SymbolDef &symbol : symbols) {
        if (symbol.name == name) {
            return &symbol;
        }
    }
    return nullptr;
}

bool SymbolTable::exists(const std::string &name) const {
    return get(name) != nullptr;
}

SymbolDef::Type SymbolTable::type(const std::string &name) const {
    const SymbolDef *symbol = get(name);
    if (!symbol) return SymbolDef::Unknown;
    return symbol->type;
}

void SymbolTable::dump(std::ostream &out) const {
    for (const SymbolDef &sym : symbols) {
        out << std::left;
        out << std::setw(10) << SymbolDef::typeName(sym.type) << ": ";
        out << std::setw(40) << sym.name << "   ";
        out << sym.origin << '\n';
    }
}

