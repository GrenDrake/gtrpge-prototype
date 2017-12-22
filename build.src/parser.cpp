#include <iostream>
#include <memory>
#include <sstream>

#include "build.h"

void Parser::checkSymbol(const Origin &origin, const std::string &name, SymbolDef::Type type) {
    for (const auto &i : symbols) {
        if (i.name == name) {
            std::stringstream ss;
            ss << "Symbol " << name << " was already defined at " << i.origin << ".";
            throw BuildError(origin, ss.str());
        }
    }
    symbols.push_back(SymbolDef(origin, name, type));
}

void Parser::parseTokens(std::list<Token>::iterator start, std::list<Token>::iterator end) {
    cur = start;

    while (cur != end) {

        if (matches("NODE")) {
            doNode();
        } else if (matches("TITLE")) {
            doTitle();
        } else if (matches("BYLINE")) {
            doByline();
        } else if (matches("VERSION")) {
            doVersion();
        } else if (matches("CONSTANT")) {
            doConstant();
        } else if (matches("ITEM")) {
            doItemDef();
        } else if (matches("SEX")) {
            doSex();
        } else if (matches("SPECIES")) {
            doSpecies();
        } else if (matches("SKILL")) {
            doSkill();
        } else if (matches("CHARACTER")) {
            doCharacter();
        } else {
            std::stringstream ss;
            throw BuildError(cur->origin, "Expected top level construct");
        }
    }

}

void Parser::doTitle() {
    require("TITLE");
    require(Token::String);
    gameData.title = gameData.addString(cur->text);
    ++cur;
    require(Token::Semicolon, true);
}

void Parser::doByline() {
    require("BYLINE");
    require(Token::String);
    gameData.byline = gameData.addString(cur->text);
    ++cur;
    require(Token::Semicolon, true);
}

void Parser::doVersion() {
    require("VERSION");
    require(Token::String);
    gameData.version = gameData.addString(cur->text);
    ++cur;
    require(Token::Semicolon, true);
}

void Parser::doConstant() {
    const Origin &origin = cur->origin;
    require("CONSTANT");
    require(Token::Identifier);
    const std::string &name = cur->text;
    checkSymbol(origin, name, SymbolDef::Constant);
    ++cur;

    require(Token::Integer);
    gameData.constants.insert(std::make_pair(name, cur->value));
    ++cur;
    require(Token::Semicolon, true);
}

void Parser::doNode() {
    const Origin &origin = cur->origin;
    require("NODE");
    require(Token::Identifier);

    std::string nodeName = cur->text;
    checkSymbol(origin, nodeName, SymbolDef::Node);

    ++cur;
    std::shared_ptr<Block> block(doBlock());
    std::shared_ptr<Node> node(new Node);
    node->name = nodeName;
    node->block = block;

    if (!gameData.nodes.insert(std::make_pair(node->name, node)).second) {
        throw BuildError(origin, "Duplicate node " + node->name);
    }
}

void Parser::doItemDef() {
    const Origin &origin = cur->origin;
    require("ITEM");
    require(Token::Identifier);
    const std::string &name = cur->text;
    checkSymbol(origin, name, SymbolDef::Item);
    ++cur;

    std::shared_ptr<ItemDef> item(new ItemDef);
    item->origin = origin;
    item->name = name;
    require(Token::OpenBrace, true);
    require(Token::String);
    item->article = gameData.addString(cur->text);
    ++cur;
    require(Token::String);
    item->singular = gameData.addString(cur->text);
    ++cur;
    require(Token::String);
    item->plural = gameData.addString(cur->text);
    ++cur;

    if (matches(Token::OpenParan)) {
        ++cur; // eat open paran
        while (!matches(Token::CloseParan)) {
            item->flags.insert(doValue());
        }
        ++cur; // eat close paran
    }

    while (!matches(Token::CloseBrace)) {
        const Origin &pOrigin = cur->origin;
        require(Token::Identifier);
        const std::string &pName = cur->text;
        ++cur;
        const Value &value = doProperty(item->name);

        if (pName == "onUse") {
            item->onUse = value;
        } else {
            throw BuildError(pOrigin, "Unknown item property " + pName);
        }
    }
    ++cur;

    if (!gameData.items.insert(std::make_pair(item->name, item)).second) {
        throw BuildError(origin, "Duplicate item " + item->name);
    }
}

void Parser::doSex() {
    const Origin &origin = cur->origin;
    require("SEX");
    require(Token::Identifier);
    const std::string &name = cur->text;
    checkSymbol(origin, name, SymbolDef::Sex);
    ++cur;

    std::shared_ptr<SexDef> sex(new SexDef);
    sex->origin = origin;
    sex->name = name;
    require(Token::OpenBrace, true);

    require(Token::String);
    sex->displayName = gameData.addString(cur->text);
    ++cur;

    if (matches(Token::OpenParan)) {
        ++cur; // eat open paran
        while (!matches(Token::CloseParan)) {
            sex->flags.insert(doValue());
        }
        ++cur; // eat close paran
    }

    require(Token::String);
    sex->subject = gameData.addString(cur->text);
    ++cur;
    require(Token::String);
    sex->object = gameData.addString(cur->text);
    ++cur;
    require(Token::String);
    sex->possess = gameData.addString(cur->text);
    ++cur;
    require(Token::String);
    sex->adject = gameData.addString(cur->text);
    ++cur;
    require(Token::String);
    sex->reflex = gameData.addString(cur->text);
    ++cur;

    require(Token::CloseBrace, true);
    if (!gameData.sexes.insert(std::make_pair(sex->name, sex)).second) {
        throw BuildError(origin, "Duplicate sex " + sex->name);
    }
}

void Parser::doSpecies() {
    const Origin &origin = cur->origin;
    require("SPECIES");
    require(Token::Identifier);
    const std::string &name = cur->text;
    checkSymbol(origin, name, SymbolDef::Species);
    ++cur;

    std::shared_ptr<SpeciesDef> species(new SpeciesDef);
    species->origin = origin;
    species->name = name;
    require(Token::OpenBrace, true);

    require(Token::String);
    species->displayName = gameData.addString(cur->text);
    ++cur;

    if (matches(Token::OpenParan)) {
        ++cur; // eat open paran
        while (!matches(Token::CloseParan)) {
            species->flags.insert(doValue());
        }
        ++cur; // eat close paran
    }

    require(Token::CloseBrace, true);
    if (!gameData.species.insert(std::make_pair(species->name, species)).second) {
        throw BuildError(origin, "Duplicate species " + species->name);
    }
}

void Parser::doSkill() {
    const Origin &origin = cur->origin;
    require("SKILL");
    require(Token::Identifier);
    const std::string &name = cur->text;
    checkSymbol(origin, name, SymbolDef::Skill);
    ++cur;

    std::shared_ptr<SkillDef> skill(new SkillDef);
    skill->origin = origin;
    skill->name = name;

    skill->statSkill = doValue();

    require(Token::String);
    skill->displayName = gameData.addString(cur->text);
    ++cur;

    skill->defaultValue = doValue();

    require(Token::Semicolon, true);
    if (!gameData.skills.insert(std::make_pair(skill->name, skill)).second) {
        throw BuildError(origin, "Duplicate skill " + skill->name);
    }
}

void Parser::doCharacter() {
    const Origin &origin = cur->origin;
    require("CHARACTER");
    require(Token::Identifier);
    const std::string &name = cur->text;
    checkSymbol(origin, name, SymbolDef::Character);
    ++cur;

    std::shared_ptr<CharacterDef> character(new CharacterDef);
    character->origin = origin;
    character->name = name;
    require(Token::OpenBrace, true);

    require(Token::String);
    character->article = gameData.addString(cur->text);
    ++cur;
    require(Token::String);
    character->displayName = gameData.addString(cur->text);
    ++cur;

    character->sex = doValue();
    character->species = doValue();

    if (matches(Token::OpenParan)) {
        ++cur; // eat open paran
        while (!matches(Token::CloseParan)) {
            character->flags.insert(doValue());
        }
        ++cur; // eat close paran
    }

    while (!matches(Token::CloseBrace)) {
        require(Token::Identifier);
        if (cur->text == "faction") {
            ++cur;
            character->faction = doValue();
        } else if (cur->text == "skills") {
            ++cur;
            require(Token::OpenParan);
            ++cur;
            while (!matches(Token::CloseParan)) {
                require(Token::Identifier);
                const std::string &name = cur->text;
                ++cur;
                const Value &v = doValue();
                require(Token::Semicolon, true);
                character->skills.insert(std::make_pair(name, v));
            }
            ++cur;
        } else if (cur->text == "gear") {
            ++cur;
            require(Token::OpenParan);
            ++cur;
            while (!matches(Token::CloseParan)) {
                require(Token::String);
                const std::string &name = gameData.addString(cur->text);
                ++cur;
                require(Token::Identifier);
                const std::string &v = cur->text;
                ++cur;
                require(Token::Semicolon, true);
                character->gear.insert(std::make_pair(name, v));
            }
            ++cur;
        } else {
            throw BuildError(origin, "Unknown character property " + cur->text);
        }
    }

    require(Token::CloseBrace, true);
    if (!gameData.characters.insert(std::make_pair(character->name, character)).second) {
        throw BuildError(origin, "Duplicate character " + character->name);
    }
}

Value Parser::doProperty(const std::string &forName) {
    const Origin &origin = cur->origin;
    if (matches(Token::Identifier)) {
        const std::string &name = cur->text;
        ++cur;
        require(Token::Semicolon, true);
        return Value(name);
    } else if (matches(Token::OpenBrace)) {
        std::stringstream ss;
        ss << "__an_" << forName << "_" << anonymousCounter;
        ++anonymousCounter;
        std::shared_ptr<Node> node(new Node);
        node->block = std::shared_ptr<Block>(doBlock());
        node->name = ss.str();
        if (!gameData.nodes.insert(std::make_pair(node->name, node)).second) {
            throw BuildError(origin, "Duplicate node " + node->name);
        }
        return Value(node->name);
    } else if (matches(Token::Integer)) {
        int v = cur->value;
        ++cur;
        require(Token::Semicolon, true);
        return Value(v);
    } else {
        throw BuildError(origin, "Expected property value.");
    }
}

std::shared_ptr<Block> Parser::doBlock() {
    std::shared_ptr<Block> block(new Block);
    require(Token::OpenBrace, true);
    while (!matches(Token::CloseBrace)) {
        std::shared_ptr<Statement> statement = doStatement();
        if (statement) {
            block->statements.push_back(statement);
        }
    }
    std::shared_ptr<Statement> doneStmt(new Statement);
    doneStmt->parts.push_back(Value("end"));
    block->statements.push_back(doneStmt);
    ++cur;
    return block;
}

std::shared_ptr<Statement> Parser::doStatement() {
    const Origin &origin = cur->origin;
    std::shared_ptr<Statement> statement(new Statement);
    statement->origin = origin;
    while (!matches(Token::Semicolon)) {
        statement->parts.push_back(doValue());
    }
    ++cur;
    if (statement->parts.empty()) {
        return nullptr;
    }
    return statement;
}

Value Parser::doValue() {
    if (matches(Token::String)) {
        std::string label = gameData.addString(cur->text);
        ++cur;
        return Value(label);
    } else if (matches(Token::Identifier)) {
        std::string label = cur->text;
        ++cur;
        return Value(label);
    } else if (matches(Token::Integer)) {
        int v = cur->value;
        ++cur;
        return v;
    } else {
        std::stringstream ss;
        ss << "Expected value type, but found " << cur->type << '.';
        throw BuildError(cur->origin, ss.str());
    }
}



void Parser::require(Token::Type type, bool advance) {
    if (cur->type != type) {
        std::stringstream ss;
        ss << "Expected " << type << " but found " << cur->type;
        throw BuildError(cur->origin, ss.str());
    }
    if (advance) ++cur;
}

void Parser::require(const std::string &text) {
    if (cur->type != Token::Identifier || cur->text != text) {
        std::stringstream ss;
        ss << "Expected \"" << text << "\" but found ";
        if (cur->type == Token::Identifier) {
            ss << '"' << cur->text << '"';
        } else {
            ss << cur->type;
        }
        ss << '.';
        throw BuildError(cur->origin, ss.str());
    }
    ++cur;
}

bool Parser::matches(Token::Type type) {
    return cur->type == type;
}

bool Parser::matches(const std::string &text) {
    return (cur->type == Token::Identifier && cur->text == text);
}


std::ostream& operator<<(std::ostream &out, const Value &type) {
    if (type.type == Value::Identifier) {
        out << "T:" << type.text;
    } else {
        out << "I:" << type.value;
    }
    return out;
}
