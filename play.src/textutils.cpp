#include <cctype>
#include <string>
#include <vector>


std::string toTitleCase(std::string text) {
    for (size_t pos = 0; pos < text.size(); ++pos) {
        if (pos == 0 || isspace(text[pos-1])) {
            text[pos] = toupper(text[pos]);
        }
    }
    return text;
}

std::string toUpperFirst(std::string text) {
    if (!text.empty()) {
        text[0] = toupper(text[0]);
    }
    return text;
}

std::string trim(std::string text) {
    if (text.empty()) return text;

    size_t pos = 0;
    while (isspace(text[pos])) ++pos;
    if (pos) text.erase(0, pos);

    pos = text.size() - 1;
    while (isspace(text[pos])) --pos;
    if (pos) text.resize(pos + 1);

    return text;
}

std::vector<std::string> explodeString(const std::string &text, int onChar) {
    std::vector<std::string> results;

    size_t pos = 0, lastpos = 0;
    pos = text.find_first_of(onChar);
    while (pos != std::string::npos) {
        std::string line = trim(text.substr(lastpos, pos-lastpos));
        if (!line.empty()) {
            results.push_back(line);
        }
        lastpos = pos + 1;
        pos = text.find_first_of(onChar, lastpos);
    }
    std::string line = trim(text.substr(lastpos, pos-lastpos));
    results.push_back(line);

    return results;
}

std::vector<std::string> wrapString(const std::string &text, unsigned width) {
    std::vector<std::string> results;

    if (text.size() < width) {
        results.push_back(text);
        return results;
    }

    size_t lastpos = 0;
    size_t pos = 0;
    while (lastpos < text.size()) {
        pos = lastpos + width;
        if (pos >= text.size()) {
            results.push_back(text.substr(lastpos));
            return results;
        }
        while (!isspace(text[pos]) && pos > lastpos) {
            --pos;
        }
        if (pos == lastpos) {
            results.push_back(text.substr(pos));
            return results;
        }

        results.push_back(text.substr(lastpos, pos-lastpos));
        lastpos = pos + 1;
    }

    return results;
}