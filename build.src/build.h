#ifndef BUILD_H
#define BUILD_H

#include <array>
#include <cstdint>
#include <fstream>
#include <list>
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

#include "errorlog.h"
#include "origin.h"
#include "builderror.h"

class SymbolTable;

#include "data.h"
class GameData {
public:
    GameData()
    : nextString(0)
    {
        skills.push_back(std::shared_ptr<SkillDef>(new SkillDef));
    }

    std::string addString(const std::string &text, SymbolTable &symbols);

    std::unordered_map<std::string, Value> constants;
    std::unordered_map<std::string, std::string> strings;
    std::vector<std::shared_ptr<Node> > nodes;
    std::vector<std::shared_ptr<SkillDef> > skills;
    std::vector<std::shared_ptr<DataType> > dataItems;
    std::string title, byline, version;
    std::vector<std::string> damageTypes;
private:
    int nextString;
};


class Token {
public:
    enum Type {
        Identifier,
        String,
        Integer,
        OpenBrace,
        CloseBrace,
        Semicolon,
        Colon,
        OpenParan,
        CloseParan,
        Indirection,
        Say
    };

    Token()
    : type(Integer), value(0)
    { }
    Token(const Origin &origin, Type type)
    : origin(origin), type(type), value(0)
    { }
    Token(const Origin &origin, Type type, const std::string &text)
    : origin(origin), type(type), text(text), value(0)
    { }
    Token(const Origin &origin, Type type, int value)
    : origin(origin), type(type), value(value)
    { }

    Origin origin;
    Type type;
    std::string text;
    int value;
};

class Lexer {
public:
    Lexer(ErrorLog &log)
    : log(log)
    { }
    void doFile(const std::string &file);

    std::list<Token> tokens;
private:
    void unescape(const Origin &origin, std::string &text);
    int here() const {
        if (pos < text.size()) {
            return text[pos];
        }
        return 0;
    }
    int prev() const {
        if (pos > 0) {
            return text[pos - 1];
        }
        return 0;
    }
    int next() {
        if (pos < text.size()) {
            if (here() == '\n') {
                ++line;
                column = 1;
            } else {
                ++column;
            }
            ++pos;
        }
        return here();
    }
    int peek() {
        if (pos + 1 >= text.size()) {
            return 0;
        }
        return text[pos + 1];
    }
    bool isIdentifier(int c, bool initial = false) {
        if (!initial && isdigit(c)) {
            return true;
        }
        if (initial && (c == '$')) {
            return true;
        }
        if (isalpha(c) || c == '-' || c == '_') {
            return true;
        }
        return false;
    }

    ErrorLog &log;
    std::string text;
    std::string file;
    std::uint32_t pos;
    int line, column;
};

struct ObjectDefSpecialization;
class Parser {
public:
    Parser(GameData &gameData, SymbolTable &symbols)
    : symbols(symbols), gameData(gameData), skillCounter(1)
    { }

    void parseTokens(std::list<Token>::iterator start, std::list<Token>::iterator end);
private:
    void doConstant();
    void doNode();
    void doSkill();
    void doDamageTypes();
    void doObject();
    std::shared_ptr<ObjectDef> doObjectCore(const Origin &origin);
    void doObjectClass(const Origin &origin, const ObjectDefSpecialization &requiredProperties);

    void doList(const std::string &myName);
    void doMap(const std::string &myName);
    Value doProperty(const std::string &forName, const std::string &propName);
    std::vector<Value> doFlags();
    std::shared_ptr<Block> doBlock();
    void doStatement(std::shared_ptr<Block> forBlock);
    Value doValue();

    void require(Token::Type type, bool advance = false);
    void require(const std::string &text);
    bool matches(Token::Type type);
    bool matches(const std::string &text);

    SymbolTable &symbols;
    std::list<Token>::iterator cur;
    GameData &gameData;
    int skillCounter;
};

std::string readFile(const std::string &file);
std::ostream& operator<<(std::ostream &out, const Token::Type &type);
std::ostream& operator<<(std::ostream &out, const Token &token);

const Command* getCommand(const std::string name);
void make_bin(GameData &gameData, const std::string &outputFile, const SymbolTable &symbols);

std::string toLowercase(std::string text);

#endif
