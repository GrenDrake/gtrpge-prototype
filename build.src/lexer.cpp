#include <iomanip>
#include <iostream>
#include <sstream>

#include "build.h"

std::string toLowercase(std::string text) {
    for (auto &c : text) {
        c = tolower(c);
    }
    return text;
}

std::ostream& operator<<(std::ostream &out, const Token::Type &type) {
    switch(type) {
        case Token::Identifier:
            out << "Identifier";
            break;
        case Token::String:
            out << "String";
            break;
        case Token::Integer:
            out << "Integer";
            break;
        case Token::Semicolon:
            out << "Semicolon";
            break;
        case Token::OpenBrace:
            out << "Open Brace";
            break;
        case Token::CloseBrace:
            out << "Close Brace";
            break;
        case Token::Colon:
            out << "Colon";
            break;
        case Token::OpenParan:
            out << "Open Paran";
            break;
        case Token::CloseParan:
            out << "Close Paran";
            break;
        case Token::Indirection:
            out << "Indirection Operator";
            break;
        default:
            out << "(unhandled type)";
    }
    return out;
}
std::ostream& operator<<(std::ostream &out, const Origin &origin) {
    out << origin.file << ':' << std::dec << origin.line << ':' << origin.column;
    return out;
}
std::ostream& operator<<(std::ostream &out, const Token &token) {
    out << '[';
    out << token.origin;
    out << ' ';
    out << token.type;
    switch(token.type) {
        case Token::Integer:
            out << ' ' << token.value;
            break;
        case Token::String:
            out << " ~" << token.text << '~';
            break;
        case Token::Identifier:
            out << ' ' << token.text;
            break;
        case Token::Semicolon:
        case Token::Colon:
        case Token::OpenBrace:
        case Token::CloseBrace:
        case Token::OpenParan:
        case Token::CloseParan:
            // do nothing
            break;
        case Token::Indirection:
            out << " * ";
            break;
        default:
            out << " (unhandled type)";
    }
    out << ']';
    return out;
}

void Lexer::unescape(const Origin &origin, std::string &text) {
    for (unsigned i = 0; i < text.size(); ++i) {
        if (text[i] != '\\') continue;

        if (i+1 >= text.size()) continue;
        switch(text[i+1]) {
            case '"':
            case '\'':
                text[i] = text[i+1];
                break;
            case 'n':
                text[i] = '\n';
                break;
            default:
                log.add(ErrorLog::Error, origin, "Unknown escape in string.");
        }
        text.erase(i+1, 1);
    }
}

void Lexer::doFile(const std::string &file) {
    tokens.clear();
    pos = 0;
    line = column = 1;

    text = readFile(file);

    while (pos < text.size()) {
        Origin origin(file, line, column);
        if (isspace(here())) {
            while (isspace(here())) {
                next();
            }
        } else if (here() == '/' && peek() == '/') {
            while (here() != '\n') {
                next();
            }
        } else if (here() == '/' && peek() == '*') {
            next(); next();
            while (here() != '*' || peek() != '/') {
                if (here() == 0) {
                    log.add(ErrorLog::Error, origin, "Unexpected end of file during block comment.");
                    return;
                }
                next();
            }
            next(); next();
        } else if (here() == ';') {
            next();
            tokens.push_back(Token(origin, Token::Semicolon));
        } else if (here() == ':') {
            next();
            tokens.push_back(Token(origin, Token::Colon));
        } else if (here() == '{') {
            next();
            tokens.push_back(Token(origin, Token::OpenBrace));
        } else if (here() == '}') {
            next();
            tokens.push_back(Token(origin, Token::CloseBrace));
        } else if (here() == '(') {
            next();
            tokens.push_back(Token(origin, Token::OpenParan));
        } else if (here() == ')') {
            next();
            tokens.push_back(Token(origin, Token::CloseParan));
        } else if (here() == '*') {
            next();
            tokens.push_back(Token(origin, Token::Indirection));
        } else if (here() == '"') {
            next();
            unsigned start = pos;
            while (here() && (here() != '"' || prev() == '\\')) {
                next();
            }
            if (!pos) {
                log.add(ErrorLog::Error, origin, "Unexpected end of file.");
                return;
            }
            std::string str = text.substr(start, pos-start);
            unescape(origin, str);
            next();
            tokens.push_back(Token(origin, Token::String, str));
        } else if (isIdentifier(here(), true) && (here() != '-' || !isdigit(peek()))) {
            unsigned start = pos;
            ++pos;
            while (isIdentifier(here())) {
                next();
            }
            std::string str = text.substr(start, pos-start);
            tokens.push_back(Token(origin, Token::Identifier, toLowercase(str)));
        } else if (here() == '0' && tolower(peek()) == 'x') {
            ++pos; ++pos;
            int value = 0;
            while (isxdigit(here())) {
                value *= 16;
                if (isdigit(here())) {
                    value += here() - '0';
                } else {
                    value += toupper(here()) - 'A' + 10;
                }
                next();
            }
            if (!isspace(here()) && !ispunct(here())) {
                log.add(ErrorLog::Error, origin, "Expected whitespace.");
            }
            tokens.push_back(Token(origin, Token::Integer, value));
        } else if (isdigit(here()) || here() == '-') {
            int value = 0;
            bool neg = false;
            if (here() == '-') {
                neg = true;
            } else {
                value = here() - '0';
            }
            while (isdigit(next())) {
                value *= 10;
                value += here() - '0';
            }
            if (!isspace(here()) && !ispunct(here())) {
                log.add(ErrorLog::Error, origin, "Expected whitespace.");
            }
            if (neg) {
                value = -value;
            }
            tokens.push_back(Token(origin, Token::Integer, value));
        } else {
            std::stringstream ss;
            ss << "Found unexpected character " << (char)here() << " (" << here() << ").";
            log.add(ErrorLog::Error, origin, ss.str());
            next();
        }
    }
}
