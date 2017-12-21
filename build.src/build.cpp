#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "build.h"


BuildError::BuildError(const std::string &msg)
: std::runtime_error(msg), errorMessage("")
{
    strncpy(errorMessage, msg.c_str(), errorMessageLength-1);
}
BuildError::BuildError(const Origin &origin, const std::string &msg)
: std::runtime_error(msg)
{
    std::stringstream ss;
    ss << origin << ' ' << msg;

    strncpy(errorMessage, ss.str().c_str(), errorMessageLength-1);
}
const char* BuildError::what() const throw() {
    return errorMessage;
}

std::string GameData::addString(const std::string &text) {
    if (strings.count(text) > 0) {
        return strings[text];
    } else {
        std::stringstream name;
        name << "__s" << nextString++;
        strings.insert(std::make_pair(text, name.str()));
        return name.str();
    }
}

std::string readFile(const std::string &file) {
    std::ifstream inf(file);
    if (!inf) {
        throw std::runtime_error("Could not open file.");
    }
    std::string content( (std::istreambuf_iterator<char>(inf)),
                         std::istreambuf_iterator<char>() );
    return content;
}

std::string formatForDump(std::string text, size_t maxlen) {
    size_t pos = text.find_first_of('\n');
    while (pos != std::string::npos) {
        text[pos] = '^';
        pos = text.find_first_of('\n');
    }
    if (text.size() >= maxlen) {
        text = text.substr(0, maxlen - 3);
        text += "...";
        return text;
    } else {
        return text;
    }
}

int main() {

    GameData gameData;
    Lexer lexer;
    try {

        lexer.doFile("gamesrc");

        Parser parser(gameData);

        parser.parseTokens(lexer.tokens.begin(), lexer.tokens.end());
    } catch (BuildError &e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    std::ofstream out("dbg_dump.txt");

    out << "FOUND " << gameData.constants.size() << " CONSTANTS:\n";
    for (auto &constant : gameData.constants) {
        out << std::setw(8) << constant.first << ": ";
        out << constant.second;
        out << "\n";
    }

    out << "\nFOUND " << gameData.strings.size() << " STRINGS:\n";
    for (auto &str : gameData.strings) {
        out << std::setw(8) << str.second << "  ~";
        out << formatForDump(str.first, 50);
        out << "~\n";
    }

    out << "\nFOUND " << gameData.items.size() << " ITEMS\n";
    for (auto &item : gameData.items) {
        out << "    " << item.first << ": ";
        out << item.second->article << ' ' << item.second->singular << ' ' << item.second->plural << "\n";
    }

    out << "\nFOUND " << gameData.nodes.size() << " NODES\n";
    for (auto &node : gameData.nodes) {
        out << "    " << node.first << '\n';
        for (auto &stmt : node.second->block->statements) {
            out << "       ";
            for (auto &text : stmt->parts) {
                out << ' ' << text;
            }
            out << ";\n";
        }
    }

    try {
        make_bin(gameData, out);
    } catch (BuildError &e) {
        std::cerr << e.what() << "\n";
    }
}