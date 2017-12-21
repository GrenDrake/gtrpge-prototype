#ifndef BUILD_H
#define BUILD_H

#include <cstdint>
#include <fstream>
#include <list>
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

class Origin {
public:
    Origin()
    : file("(no-file)"), line(0), column(0)
    { }
    Origin(const std::string &file, int line, int column)
    : file(file), line(line), column(column)
    { }

    std::string file;
    int line, column;
};

class BuildError : public std::runtime_error {
public:
    BuildError(const std::string &msg);
    BuildError(const Origin &origin, const std::string &msg);
    virtual const char* what() const throw();
private:
    static const int errorMessageLength = 128;
    char errorMessage[errorMessageLength];
};

#include "data.h"
class GameData {
public:
    GameData()
    : nextString(0)
    { }

    std::string addString(const std::string &text);

    std::unordered_map<std::string, std::uint32_t> constants;
    std::unordered_map<std::string, std::string> strings;
    std::unordered_map<std::string, std::shared_ptr<Node> > nodes;
    std::string title, byline, version;
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
        Colon
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
        if (isalpha(c) || c == '-' || c == '_') {
            return true;
        }
        return false;
    }

    std::string text;
    std::string file;
    std::uint32_t pos;
    int line, column;
};

class SymbolDef {
public:
    enum Type {
        Node, Constant
    };

    SymbolDef(const Origin &origin, const std::string &name, Type type)
    : origin(origin), name(name), type(type)
    { }

    Origin origin;
    std::string name;
    Type type;
};
class Parser {
public:
    Parser(GameData &gameData)
    : gameData(gameData)
    { }

    void parseTokens(std::list<Token>::iterator start, std::list<Token>::iterator end);
private:
    void doTitle();
    void doByline();
    void doVersion();
    void doConstant();
    void doNode();
    std::shared_ptr<Block> doBlock();
    std::shared_ptr<Statement> doStatement();

    void require(Token::Type type, bool advance = false);
    void require(const std::string &text);
    bool matches(Token::Type type);
    bool matches(const std::string &text);

    void checkSymbol(const Origin &origin, const std::string &name, SymbolDef::Type type);

    std::vector<SymbolDef> symbols;
    std::list<Token>::iterator cur;
    GameData &gameData;
};

std::string readFile(const std::string &file);
std::ostream& operator<<(std::ostream &out, const Token::Type &type);
std::ostream& operator<<(std::ostream &out, const Origin &origin);
std::ostream& operator<<(std::ostream &out, const Token &token);

void make_bin(GameData &gameData, std::ostream &dbgout);

#endif
