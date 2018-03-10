#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <map>

#include "build.h"
#include "symboltable.h"
#include "../play.src/constants.h"

static std::string mangleLabel(const std::string &nodeName, const std::string &original) {
    std::string labelName = "__lbl_"+nodeName+"_"+original;
    return labelName;
}

static std::map<std::string, unsigned> labels;

unsigned getLabel(const std::string &name) {
    const auto &v = labels.find(name);
    if (v != labels.end()) {
        return v->second;
    }
    return 0;
}

uint32_t processValue(const Origin &origin, const Value &value, const std::string &nodeName) {
    switch(value.type) {
        case Value::Integer:
            return value.value;
        case Value::Global:
        case Value::Identifier: {
            const auto &v = labels.find(value.text);
            if (v != labels.end()) {
                return v->second;
            }

            if (nodeName != "") {
                std::string mangled = mangleLabel(nodeName, value.text);
                const auto &mv = labels.find(mangled);
                if (mv != labels.end()) {
                    return mv->second;
                }
            }
            std::cerr << "WARNING: " << origin << " Unknown symbol " << value.text << '\n';
            return 0; }
        case Value::FlagSet: {
            std::uint32_t result = 0;

            if (!value.mFlagSet.empty()) {
                for (auto &flg : value.mFlagSet) {
                    std::uint32_t value = processValue(origin, flg, "");
                    result |= value;
                }
            }
            return result; }
    }
    return 0;
}

void writeByte(std::ostream &out, std::uint8_t value) {
    out.write((const char *)&value, sizeof(value));
}

void writeShort(std::ostream &out, std::uint16_t value) {
    out.write((const char *)&value, sizeof(value));
}

void writeWord(std::ostream &out, std::uint32_t value) {
    out.write((const char *)&value, sizeof(value));
}

void writeValue(std::ostream &out, const Origin &origin, const Value &value) {
    std::uint32_t result = processValue(origin, value, "");
    writeWord(out, result);
}

void writeFlags(std::ostream &out, const Origin &origin, const std::vector<Value> &flags) {
    std::uint32_t result = 0;

    if (!flags.empty()) {
        for (auto &flg : flags) {
            std::uint32_t value = processValue(origin, flg, "");
            result |= value;
        }
    }

    writeWord(out, result);
}

void writeLabelValue(std::ostream &out, const std::string &labelName) {
    std::uint32_t v = labels[labelName];
    out.write((const char *)&v, sizeof(std::uint32_t));
}

template<class T>
static void doPositioning(std::map<std::string, unsigned> &labels, std::uint32_t &position, std::vector<std::shared_ptr<T> > data) {
    for (std::shared_ptr<T> &c : data) {
        labels.insert(std::make_pair(c->name, position));
        c->pos = position;
        position += c->getSize();
    }
}

void make_bin(GameData &gameData, const std::string &outputFile, const SymbolTable &symbols) {
    // if (gameData.nodes.count("start") == 0) {
    //     throw BuildError("Game lacks \"start\" node.");
    // }

    std::fstream out(outputFile, std::ios::out | std::ios::in | std::ios::binary | std::ios::trunc);
    if (!out) {
        throw BuildError("Could not open output file.");
    }

    // add space for header
    out.put('G');   out.put('R');
    out.put('P');   out.put('G');
    out.put(0);     out.put(0);
    out.put(1);     out.put(0);
    for (int i = 0; i < headerSize - 8; ++i) {
        out.put(0);
    }

    // setup the initial, default labels as well as the ones created by constants
    std::uint32_t pos = headerSize;
    labels.insert(std::make_pair("true", 1));
    labels.insert(std::make_pair("false", 0));
    labels.insert(std::make_pair("pro-subject", propSubject));
    labels.insert(std::make_pair("pro-object", propObject));
    labels.insert(std::make_pair("pro-possess", propPossessive));
    labels.insert(std::make_pair("pro-adject", propAdjective));
    labels.insert(std::make_pair("pro-reflex", propReflexive));
    // setup the labels for the temp storage values
    for (unsigned i = 0; i < storageTempCount; ++i) {
        std::stringstream ss;
        ss << "_" << i;
        labels.insert(std::make_pair(ss.str(), storageFirstTemp-i));
    }

    // write the string table and create the appropriate labels
    std::uint8_t idByte = idString;
    for (auto &str : gameData.strings) {
        labels.insert(std::make_pair(str.second, pos));
        pos += str.first.size() + 2;
        out.write(reinterpret_cast<char*>(&idByte), 1);
        out.write(str.first.c_str(), str.first.size());
        out.put(0);
    }

    for (auto &c : gameData.constants) {
        if (c.second.type == Value::Identifier) {
            auto iter = labels.find(c.second.text);
            if (iter == labels.end()) {
                throw BuildError(Origin(), "Bad constant value");
            }
            labels.insert(std::make_pair(c.first, iter->second));
        } else {
            labels.insert(std::make_pair(c.first, c.second.value));
        }
    }

    // reserve space for the skill table
    labels.insert(std::make_pair("__skill_table", pos));
    pos += sklSize * sklCount;

    // reserve space for the damage types list
    labels.insert(std::make_pair("__damage_types", pos));
    pos += damageTypeCount * 4;

    // position remaining game data
    doPositioning(labels, pos, gameData.dataItems);

    for (auto &node : gameData.nodes) {
        const std::string &nodeName = node->name;
        labels.insert(std::make_pair(nodeName, pos));
        ++pos;
        for (auto &stmt : node->block->statements) {
            stmt->pos = pos;
            if (stmt->parts.empty()) continue;

            if (stmt->parts.front().type != Value::Identifier) {
                throw BuildError(stmt->origin, "Command must be identifier");
            }

            unsigned size = 0;
            if (stmt->parts.front().text == "label") {
                std::string mangledName = mangleLabel(nodeName, stmt->parts.back().text);
                if (labels.find(mangledName) == labels.end()) {
                    labels.insert(std::make_pair(mangledName, pos));
                } else {
                    std::stringstream errorMessage;
                    errorMessage << "Duplicate label ~" << stmt->parts.back().text;
                    errorMessage << "~ in node ~" << nodeName << "~.";
                    throw BuildError(node->origin, errorMessage.str());
                }
            } else {
                size = 1 + (stmt->parts.size() - 1) * 4;
            }
            pos += size;
        }
    }

    // write skill table
    for (int i = 0; i < sklCount; ++i) {
        std::shared_ptr<SkillDef> skill;
        if (i < static_cast<int>(gameData.skills.size())) {
            skill = gameData.skills[i];
        } else {
            skill = gameData.skills[0];
        }
        writeValue(out, skill->origin, skill->statSkill);
        writeLabelValue(out, skill->displayName);
        writeFlags(out, skill->origin, skill->flags);
        writeWord(out, skill->defaultValue);
        writeWord(out, skill->recoveryRate);
    }

    // write damage type list
    for (unsigned i = 0; i < damageTypeCount; ++i) {
        if (i < gameData.damageTypes.size()) {
            writeLabelValue(out, gameData.damageTypes[i]);
        } else {
            writeWord(out, 0);
        }
    }

    // write remaining game data
    for (auto &dataItem : gameData.dataItems) {
        dataItem->write(out, symbols);
    }

    idByte = idNode;
    for (auto &node : gameData.nodes) {
        const std::string &nodeName = node->name;
        out.write(reinterpret_cast<char*>(&idByte), 1);
        for (auto &stmt : node->block->statements) {
            const std::string &cmdName = stmt->parts.front().text;
            const Command *cmd = getCommand(cmdName);
            if (!cmd) {
                throw BuildError(stmt->origin, "Unknown command " + cmdName);
            }
            if (cmd->code < 0) continue;

            out.put(cmd->code);
            auto cur = stmt->parts.begin();
            ++cur;
            while (cur != stmt->parts.end()) {
                uint32_t v = processValue(stmt->origin, *cur, nodeName);
                out.write((const char *)&v, 4);
                ++cur;
            }
        }
    }

    out.seekp(headerStartNode);
    writeLabelValue(out, "start");
    writeLabelValue(out, "title");
    writeLabelValue(out, "byline");
    writeLabelValue(out, "version");
    writeLabelValue(out, "__skill_table");
    writeLabelValue(out, "__damage_types");
    SymbolTable junkTable;
    writeLabelValue(out, gameData.addString("weapon", junkTable));

    time_t theTime = time(nullptr);
    struct tm *aTime = localtime(&theTime);
    std::uint32_t v;
    v = (aTime->tm_year + 1900) * 10000;
    v += (aTime->tm_mon + 1) * 100;
    v += (aTime->tm_mday);
    writeWord(out, v);

    std::cerr << "Created " << outputFile << ".\n";

    std::ofstream labelFile("dbg_labels.txt");
    labelFile << "LABELS (" << labels.size() << "):\n" << std::hex << std::setfill('0');
    for (auto &label : labels) {
        labelFile << "0x" << std::setw(8) << label.second << ": " << label.first << '\n';
    }
}