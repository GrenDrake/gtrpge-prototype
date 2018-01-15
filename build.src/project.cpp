#include <cctype>
#include <fstream>
#include <iostream>
#include <list>
#include <string>

#include "project.h"


std::list<std::string> simpleParse(const std::string &text) {
    std::list<std::string> tokens;

    unsigned pos = 0;
    while (pos < text.size()) {
        if (isspace(text[pos])) {
            while (pos < text.size() && isspace(text[pos])) {
                ++pos;
            }
            continue;
        }

        if (text[pos] == '"') {
            ++pos;
            unsigned start = pos;
            while (pos < text.size() && text[pos] != '"') {
                ++pos;
            }
            tokens.push_back(text.substr(start, pos-start));
        } else {
            unsigned start = pos;
            while (pos < text.size() && !isspace(text[pos])) {
                ++pos;
            }
            tokens.push_back(text.substr(start, pos-start));
        }
    }

    return tokens;
}


ProjectFile* load_project(const char *project_file) {
    std::ifstream inf(project_file);
    if (!inf) {
        std::cerr << "Error opening project file " << project_file << "\n";
        return nullptr;
    }

    ProjectFile *pf = new ProjectFile;
    std::string line;

    while (std::getline(inf, line)) {
        auto tokens = simpleParse(line);
        if (tokens.empty()) continue;
        std::string what = tokens.front();
        tokens.pop_front();
        if (what == "files") {
            for (const std::string &file : tokens) {
                pf->sourceFiles.push_back(file);
            }
        } else if (what == "output") {
            if (tokens.size() != 1) {
                std::cerr << "Output must specify exactly one filename.\n";
                delete pf;
                return nullptr;
            }
            pf->outputFile = tokens.front();
        } else {
            std::cout << "Items: " << tokens.size() << "\n";
            for (const std::string &s : tokens) {
                std::cout << "   -" << s << "-\n";
            }
        }
    }

    return pf;
}