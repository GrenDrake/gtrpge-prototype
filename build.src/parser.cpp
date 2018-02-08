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

struct ObjectDefSpecialization {
    std::string name;
    int classId;
    const char *requiredProperties[10];
};

void Parser::parseTokens(std::list<Token>::iterator start, std::list<Token>::iterator end) {
    cur = start;
    ObjectDefSpecialization objectTypes[] = {
        { "sex",       ocSex,       { "name", "object", "reflexive", "adjective", "possessive" } },
        { "species",   ocSpecies,   { "name" } },
        { "action",    ocAction,    { "name" } },
        { "item",      ocItem,      { "article", "name", "plural" } },
        { "character", ocCharacter, { "article", "name", "sex", "species" } },
        { "" }
    };

    while (cur != end) {
        const Origin &origin = cur->origin;

        if (!matches(Token::Identifier)) {
            std::stringstream ss;
            ss << "Expected identifier at top level, but found " << cur->type << ".";
            throw BuildError(origin, ss.str());
        }

        if (matches("node")) {
            doNode();
        } else if (matches("constant")) {
            doConstant();
        } else if (matches("skill")) {
            doSkill();
        } else if (matches("damage-types")) {
            doDamageTypes();
        } else if (matches("object")) {
            doObject();
        } else {
            bool foundType = false;

            for (const auto &type : objectTypes) {
                if (cur->text == type.name) {
                    ++cur;
                    doObjectClass(origin, type);
                    foundType = true;
                    break;
                }
            }

            if (!foundType) {
                std::stringstream ss;
                ss << "Expected top level construct, but found " << cur->type;
                if (cur->type == Token::Identifier) {
                    ss << " ~" << cur->text << '~';
                }
                throw BuildError(origin, ss.str());
            }
        }
    }
}


void Parser::doConstant() {
    const Origin &origin = cur->origin;
    require("constant");
    require(Token::Identifier);
    const std::string &name = cur->text;
    checkSymbol(origin, name, SymbolDef::Constant);
    ++cur;

    if (matches(Token::Integer)) {
        gameData.constants.insert(std::make_pair(name, Value(cur->value)));
    } else if (matches(Token::String)) {
        std::string labelName = gameData.addString(cur->text);
        gameData.constants.insert(std::make_pair(name, Value(labelName)));
    } else {
        throw BuildError(origin, "Constant value must be integer literal or string.");
    }
    ++cur;
    require(Token::Semicolon, true);
}

void Parser::doNode() {
    const Origin &origin = cur->origin;
    require("node");
    require(Token::Identifier);

    std::string nodeName = cur->text;
    checkSymbol(origin, nodeName, SymbolDef::Node);

    ++cur;
    std::shared_ptr<Block> block(doBlock());
    std::shared_ptr<Node> node(new Node);
    node->name = nodeName;
    node->block = block;
    node->origin = origin;

    gameData.nodes.push_back(node);
}

void Parser::doDamageTypes() {
    const Origin &origin = cur->origin;
    require("damage-types");
    require(Token::OpenBrace, true);
    unsigned typeNumber = 0;
    while (!matches(Token::CloseBrace)) {
        require(Token::Identifier);
        const std::string &typeName = cur->text;
        ++cur;
        require(Token::String);
        std::string typeRef = gameData.addString(cur->text);
        ++cur;

        checkSymbol(origin, typeName, SymbolDef::DamageType);
        gameData.constants.insert(std::make_pair(typeName, Value(typeNumber++)));
        gameData.damageTypes.push_back(typeRef);
    }
    ++cur;
}

void Parser::doObject() {
    const Origin &origin = cur->origin;
    require("object");
    std::shared_ptr<ObjectDef> obj = doObjectCore(origin);

    gameData.dataItems.push_back(obj);
}

std::shared_ptr<ObjectDef> Parser::doObjectCore(const Origin &origin) {
    require(Token::Identifier);
    std::shared_ptr<ObjectDef> obj(new ObjectDef);
    obj->origin = origin;
    obj->name = cur->text;
    ++cur;
    require(Token::OpenBrace, true);
    while (!matches(Token::CloseBrace)) {
        require(Token::Identifier);
        const std::string &propName = cur->text;
        ++cur;

        Value value = doProperty(obj->name);

        std::uint32_t propId = ObjectDef::getPropertyIdent(propName);
        if (propId == propInternalName) {
            throw BuildError(origin, "Cannot set object internal name");
        }
        auto iter = obj->properties.find(propId);
        if (iter != obj->properties.end()) {
            throw BuildError(origin, "Duplicate property " + propName);
        } else {
            obj->properties.insert(std::make_pair(propId, value));
        }
    }
    ++cur;


    obj->properties.insert(std::make_pair(propInternalName, Value(gameData.addString(obj->name))));
    return obj;
}

void Parser::doObjectClass(const Origin &origin, const ObjectDefSpecialization &def) {
    std::shared_ptr<ObjectDef> newObj = doObjectCore(origin);
    newObj->properties.insert(std::make_pair(propClass, Value(def.classId)));

    for (int i = 0; def.requiredProperties[i] != nullptr; ++i) {
        const char *name = def.requiredProperties[i];
        if (!newObj->hasProperty(name)) {
            std::stringstream ss;
            ss << "Object " << newObj->name << " requires property " << name;
            throw BuildError(newObj->origin, ss.str());
        }
    }

    gameData.dataItems.push_back(newObj);
}


std::string Parser::doList() {
    const Origin &origin = cur->origin;
    std::stringstream ss;
    ss << "__an_list_" << anonymousCounter;
    ++anonymousCounter;

    require(Token::OpenParan, true);
    std::shared_ptr<DataList> list(new DataList);
    list->origin = origin;
    list->name = ss.str();
    while (!matches(Token::CloseParan)) {
        Value v = doValue();
        list->values.push_back(v);
    }
    ++cur;

    gameData.dataItems.push_back(list);
    return ss.str();
}

std::string Parser::doSkillSet(bool setDefaults) {
    const Origin &origin = cur->origin;
    std::stringstream ss;
    ss << "__an_skillset_" << anonymousCounter;
    ++anonymousCounter;

    require(Token::OpenParan, true);
    std::shared_ptr<SkillSet> skillset(new SkillSet);
    skillset->origin = origin;
    skillset->name = ss.str();
    skillset->setDefaults = setDefaults;
    while (!matches(Token::CloseParan)) {
        require(Token::Identifier);
        const std::string &name = cur->text;
        ++cur;
        
        if (name == "defaults") {
            skillset->setDefaults = true;
            require(Token::Semicolon, true);
            continue;
        }
        
        const Value &v = doValue();
        require(Token::Semicolon, true);
        skillset->skillMap.insert(std::make_pair(name, v));
    }
    ++cur;

    gameData.dataItems.push_back(skillset);
    return ss.str();
}

void Parser::doSkill() {
    const Origin &origin = cur->origin;
    require("skill");
    require(Token::Identifier);
    const std::string &name = cur->text;
    checkSymbol(origin, name, SymbolDef::Skill);
    ++cur;

    // create constant with skill name
    gameData.constants.insert(std::make_pair(name, Value(skillCounter++)));

    std::shared_ptr<SkillDef> skill(new SkillDef);
    skill->origin = origin;
    skill->name = name;

    skill->statSkill = doValue();

    require(Token::String);
    skill->displayName = gameData.addString(cur->text);
    ++cur;

    require(Token::Integer);
    skill->defaultValue = cur->value;
    ++cur;

    skill->flags = doFlags();

    require(Token::Semicolon, true);
    gameData.skills.push_back(skill);
}

Value Parser::doProperty(const std::string &forName) {
    const Origin &origin = cur->origin;
    if (matches(Token::String)) {
        std::string label = gameData.addString(cur->text);
        ++cur;
        require(Token::Semicolon, true);
        return Value(label);
    } else if (matches(Token::Identifier)) {
        std::string &name = cur->text;
        ++cur;
        if (matches(Token::OpenParan)) {
            if (name == "list") {
                name = doList();
            } else if (name == "skillset") {
                name = doSkillSet();
            } else {
                throw BuildError(origin, "Unknown property type name " + name + ".");
            }
        } else if (name[0] == '$') {
            std::uint16_t ident = ObjectDef::getPropertyIdent(name.substr(1));
            require(Token::Semicolon, true);
            return Value(ident);
        }
        require(Token::Semicolon, true);
        return Value(name);
    } else if (matches(Token::OpenBrace)) {
        std::stringstream ss;
        ss << "__an_" << forName << "_" << anonymousCounter;
        ++anonymousCounter;
        std::shared_ptr<Node> node(new Node);
        node->block = std::shared_ptr<Block>(doBlock());
        node->name = ss.str();
        gameData.nodes.push_back(node);
        require(Token::Semicolon, true);
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

std::vector<Value> Parser::doFlags() {
    std::vector<Value> flags;

    if (!matches(Token::OpenParan)) {
        return flags;
    }
    ++cur;

    while (!matches(Token::CloseParan)) {
        flags.push_back(doValue());
    }
    ++cur;

    return flags;
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
    if (matches(Token::String)) {
        return doImpliedSay();
    }

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

    const Value &cmdName = statement->parts.front();
    if (cmdName.type != Value::Identifier) {
        throw BuildError(origin, "Command must be identifier");
    }

    statement->commandInfo = getCommand(cmdName.text);
    if (!statement->commandInfo) {
        throw BuildError(origin, "Unknown command " + cmdName.text);
    }

    if (statement->parts.size() != (unsigned)statement->commandInfo->argCount + 1) {
        std::stringstream ss;
        ss << "Command " << cmdName.text << " expects " << statement->commandInfo->argCount;
        ss << " arguments, but " << (statement->parts.size() - 1) << " were found.";
        throw BuildError(origin, ss.str());
    }


    return statement;
}

std::shared_ptr<Statement> Parser::doImpliedSay() {
    const Origin &origin = cur->origin;
    std::shared_ptr<Statement> statement(new Statement);
    statement->origin = origin;

    statement->parts.push_back(Value("say"));
    statement->parts.push_back(doValue());
    require(Token::Semicolon, true);
    return statement;
}

Value Parser::doValue() {
    if (matches(Token::String)) {
        std::string label = gameData.addString(cur->text);
        ++cur;
        return Value(label);
    } else if (matches(Token::Identifier)) {
        std::string label;
        if (cur->text[0] == '#') {
            std::string label = cur->text.substr(1);
            ++cur;
            return Value(Value::Global, label);
        } else if (cur->text[0] == '$') {
            std::uint16_t ident = ObjectDef::getPropertyIdent(cur->text.substr(1));
            ++cur;
            return Value(ident);
        } else {
            std::string label = cur->text;
            ++cur;
            return Value(Value::Identifier, label);
        }
    } else if (matches(Token::Integer)) {
        int v = cur->value;
        ++cur;
        return Value(v);
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
