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

        if (matches("node")) {
            doNode();
        } else if (matches("title")) {
            doTitle();
        } else if (matches("byline")) {
            doByline();
        } else if (matches("version")) {
            doVersion();
        } else if (matches("constant")) {
            doConstant();
        } else if (matches("item")) {
            doItemDef();
        } else if (matches("sex")) {
            doSex();
        } else if (matches("species")) {
            doSpecies();
        } else if (matches("skill")) {
            doSkill();
        } else if (matches("character")) {
            doCharacter();
        } else if (matches("action")) {
            doAction();
        } else if (matches("damage-types")) {
            doDamageTypes();
        } else if (matches("object")) {
            doObject();
        } else {
            std::stringstream ss;
            ss << "Expected top level construct, but found " << cur->type;
            if (cur->type == Token::Identifier) {
                ss << " ~" << cur->text << '~';
            }
            throw BuildError(cur->origin, ss.str());
        }
    }

}

void Parser::doTitle() {
    require("title");
    require(Token::String);
    gameData.title = gameData.addString(cur->text);
    ++cur;
    require(Token::Semicolon, true);
}

void Parser::doByline() {
    require("byline");
    require(Token::String);
    gameData.byline = gameData.addString(cur->text);
    ++cur;
    require(Token::Semicolon, true);
}

void Parser::doVersion() {
    require("version");
    require(Token::String);
    gameData.version = gameData.addString(cur->text);
    ++cur;
    require(Token::Semicolon, true);
}

void Parser::doConstant() {
    const Origin &origin = cur->origin;
    require("constant");
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

void Parser::doItemDef() {
    const Origin &origin = cur->origin;
    require("item");
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

    if (matches(Token::String)) {
        item->description = gameData.addString(cur->text);
        ++cur;
    }

    item->flags = doFlags();

    while (!matches(Token::CloseBrace)) {
        const Origin &pOrigin = cur->origin;
        require(Token::Identifier);
        const std::string &pName = cur->text;
        ++cur;

        if (pName == "onuse") {
            const Value &value = doProperty(item->name);
            item->onUse = value;
        } else if (pName == "canequip") {
            const Value &value = doProperty(item->name);
            item->canEquip = value;
        } else if (pName == "onequip") {
            const Value &value = doProperty(item->name);
            item->onEquip = value;
        } else if (pName == "onremove") {
            const Value &value = doProperty(item->name);
            item->onRemove = value;
        } else if (pName == "slot") {
            const Value &value = doValue();
            item->slot = value;
        } else if (pName == "actionlist") {
            item->actionsList = doList();
        } else if (pName == "skills") {
            item->skillSet = doSkillSet();
        } else {
            throw BuildError(pOrigin, "Unknown item property " + pName);
        }
    }
    ++cur;

    gameData.dataItems.push_back(item);
}

void Parser::doAction() {
    const Origin &origin = cur->origin;
    require("action");

    require(Token::Identifier);
    const std::string &name = cur->text;
    checkSymbol(origin, name, SymbolDef::Item);
    ++cur;

    require(Token::OpenBrace, true);

    std::shared_ptr<ActionDef> action(new ActionDef);
    action->origin = origin;
    action->name = name;

    require(Token::String);
    action->displayName = gameData.addString(cur->text);
    ++cur;

    while (!matches(Token::CloseBrace)) {
        require(Token::Identifier);
        const std::string &pName = cur->text;
        ++cur;

        if (pName == "cost") {
            require(Token::Identifier);
            action->energyStat = cur->text;
            ++cur;

            require(Token::Integer);
            action->energyCost = cur->value;
            ++cur;
        } else if (pName == "incombat") {
            action->combatNode = doProperty(action->name);
        } else if (pName == "outofcombat") {
            action->peaceNode = doProperty(action->name);
        } else {
            throw BuildError(origin, "Unknown action property " + pName);
        }
    }
    ++cur;

    gameData.dataItems.push_back(action);
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
        gameData.constants.insert(std::make_pair(typeName, typeNumber++));
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
        auto iter = obj->properties.find(propId);
        if (iter != obj->properties.end()) {
            throw BuildError(origin, "Duplicate property " + propName);
        } else {
            obj->properties.insert(std::make_pair(propId, value));
        }
    }
    ++cur;
    return obj;
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
        const Value &v = doValue();
        require(Token::Semicolon, true);
        skillset->skillMap.insert(std::make_pair(name, v));
    }
    ++cur;

    gameData.dataItems.push_back(skillset);
    return ss.str();
}


void Parser::doSex() {
    const Origin &origin = cur->origin;
    require("sex");
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

    sex->flags = doFlags();

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
    gameData.dataItems.push_back(sex);
}

void Parser::doSpecies() {
    const Origin &origin = cur->origin;
    require("species");
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

    species->flags = doFlags();

    require(Token::CloseBrace, true);
    gameData.dataItems.push_back(species);
}

void Parser::doSkill() {
    const Origin &origin = cur->origin;
    require("skill");
    require(Token::Identifier);
    const std::string &name = cur->text;
    checkSymbol(origin, name, SymbolDef::Skill);
    ++cur;

    // create constant with skill name
    gameData.constants.insert(std::make_pair(name, skillCounter++));

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

void Parser::doCharacter() {
    const Origin &origin = cur->origin;
    require("character");
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

    character->flags = doFlags();

    while (!matches(Token::CloseBrace)) {
        require(Token::Identifier);
        const std::string &identText = cur->text;
        ++cur;
        if (identText == "faction") {
            character->faction = doValue();
        } else if (identText == "skills") {
            character->skillSet = doSkillSet(true);
        } else if (identText == "gear") {
            character->gearList = doList();
        } else if (identText == "baseabilities") {
            character->baseAbilities = doList();
        } else if (identText == "extraabilities") {
            character->extraAbilities = doList();
        } else {
            throw BuildError(origin, "Unknown character property " + identText);
        }
    }

    require(Token::CloseBrace, true);
    gameData.dataItems.push_back(character);
}

Value Parser::doProperty(const std::string &forName) {
    const Origin &origin = cur->origin;
    if (matches(Token::String)) {
        std::string label = gameData.addString(cur->text);
        ++cur;
        return Value(label);
    } else if (matches(Token::Identifier)) {
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
        gameData.nodes.push_back(node);
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

std::unordered_set<Value> Parser::doFlags() {
    std::unordered_set<Value> flags;

    if (!matches(Token::OpenParan)) {
        return flags;
    }
    ++cur;

    while (!matches(Token::CloseParan)) {
        flags.insert(doValue());
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
        } else {
            std::string label = cur->text;
            ++cur;
            return Value(Value::Identifier, label);
        }
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
