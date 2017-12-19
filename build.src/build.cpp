#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "build.h"


BuildError::BuildError(const std::string &msg)
: std::runtime_error(msg), errorMessage("")
{
    strncpy(errorMessage, msg.c_str(), 63);
}
BuildError::BuildError(const Origin &origin, const std::string &msg)
: std::runtime_error(msg)
{
    std::stringstream ss;
    ss << origin << ' ' << msg;

    strncpy(errorMessage, ss.str().c_str(), 63);
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

int main() {

    GameData gameData;
    try {
        Lexer lexer;

        lexer.doFile("gamesrc");

        Parser parser(gameData);

        parser.parseTokens(lexer.tokens.begin(), lexer.tokens.end());
    } catch (BuildError &e) {
        std::cerr << e.what() << "\n";
    }


    std::cout << "FOUND " << gameData.strings.size() << " STRINGS:\n";
    for (auto &str : gameData.strings) {
        std::cout << std::setw(8) << str.second << "  ~";
        if (str.first.size() > 50) {
            std::cout << str.first.substr(0,47) << "...";
        } else {
            std::cout << str.first;
        }
        std::cout << "~\n";
    }

    std::cout << "\nFOUND " << gameData.nodes.size() << " NODES\n";
    for (auto &node : gameData.nodes) {
        std::cout << "    " << node.first << '\n';
        for (auto &stmt : node.second->block->statements) {
            std::cout << "       ";
            for (auto &text : stmt->parts) {
                std::cout << ' ' << text;
            }
            std::cout << ";\n";
        }
    }

    try {
        make_bin(gameData);
    } catch (BuildError &e) {
        std::cerr << e.what() << "\n";
    }
}