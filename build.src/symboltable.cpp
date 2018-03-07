#include <string>
#include <sstream>

#include "builderror.h"
#include "symboltable.h"

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
    if (!symbol) return SymbolDef::None;
    return symbol->type;
}
