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
        if (matches(Token::String)) {
            std::string label = gameData.addString(cur->text);
            statement->parts.push_back(Value(label));
            ++cur;
        } else if (matches(Token::Identifier)) {
            statement->parts.push_back(Value(cur->text));
            ++cur;
        } else if (matches(Token::Integer)) {
            statement->parts.push_back(Value(cur->value));
            ++cur;
        } else {
            std::stringstream ss;
            ss << "Unexpected " << cur->type << '.';
            throw BuildError(origin, ss.str());
        }
    }
    ++cur;
    if (statement->parts.empty()) {
        return nullptr;
    }
    return statement;
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
