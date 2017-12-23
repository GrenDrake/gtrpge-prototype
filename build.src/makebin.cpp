#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "build.h"
#include "../play.src/constants.h"

class Command {
public:
    const char *text;
    int code;
    int argCount;
};

Command commands[] = {
    { "label",           -1,                1 },
    { "end",             opEnd,             0 },
    { "do-node",         opDoNode,          1 },
    { "has-flag",        opHasFlag,         2 },
    { "push",            opPush,            1 },
    { "pop",             opPop,             0 },
    { "set-location",    opSetLocation,     1 },
    { "add-option",      opAddOption,       2 },
    { "add-option-xtra", opAddOptionXtra,   3 },
    { "add-continue",    opAddContinue,     1 },
    { "add-return",      opAddReturn,       0 },
    { "say",             opSay,             1 },
    { "say-number",      opSayNumber,       1 },
    { "jump",            opJump,            1 },
    { "jump-true",       opJumpTrue,        1 },
    { "jump-false",      opJumpFalse,       1 },
    { "jump-eq",         opJumpEq,          2 },
    { "jump-neq",        opJumpNeq,         2 },
    { "jump-lt",         opJumpLt,          2 },
    { "jump-lte",        opJumpLte,         2 },
    { "jump-gt",         opJumpGt,          2 },
    { "jump-gte",        opJumpGte,         2 },
    { "store",           opStore,           2 },
    { "fetch",           opFetch,           1 },
    { "add-items",       opAddItems,        2 },
    { "remove-items",    opRemoveItems,     2 },
    { "item-qty",        opItemQty,         1 },
    { "increment",       opIncrement,       0 },
    { "decrement",       opDecrement,       0 },
};
static const Command* getCommand(const std::string name) {
    for (Command &cmd : commands) {
        if (name == cmd.text) {
            return &cmd;
        }
    }
    return nullptr;
}

static std::string mangleLabel(const std::string &nodeName, const std::string &original) {
    std::string labelName = "__n_"+nodeName+"_"+original;
    return labelName;
}


static uint32_t processValue(const Origin &origin, const std::unordered_map<std::string, unsigned> &labels, const Value &value, const std::string &nodeName) {
    switch(value.type) {
        case Value::Integer:
            return value.value;
        case Value::Identifier:
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
            return 0;
    }
}

static void writeWord(std::ostream &out, std::uint32_t word) {
    out.write((const char *)&word, sizeof(std::uint32_t));
}

static void writeValue(std::ostream &out, const Origin &origin, const std::unordered_map<std::string, unsigned> &labels, const Value &value) {
    std::uint32_t result = processValue(origin, labels, value, "");
    out.write((const char *)&result, sizeof(std::uint32_t));
}

static void writeFlags(std::ostream &out,
                                const Origin &origin,
                                  const std::unordered_map<std::string, unsigned> &labels,
                                  const std::unordered_set<Value> &flags) {
    std::uint32_t result = 0;

    if (!flags.empty()) {
        for (auto &flg : flags) {
            std::uint32_t flagNo = processValue(origin, labels, flg, "");
            if (flagNo >= 32) {
                throw BuildError(origin, "Flag values must be in range (0-31).");
            }
            std::uint32_t fv = 1 << flagNo;
            result |= fv;
        }
    }

    out.write((const char *)&result, sizeof(std::uint32_t));
}

static void writeLabelValue(std::ostream &out, std::unordered_map<std::string, unsigned> &labels, const std::string &labelName) {
    std::uint32_t v = labels[labelName];
    out.write((const char *)&v, sizeof(std::uint32_t));
}

template<class T>
static void doPositioning(std::unordered_map<std::string, unsigned> &labels, std::uint32_t &position, std::vector<std::shared_ptr<T> > data) {
    for (std::shared_ptr<T> &c : data) {
        labels.insert(std::make_pair(c->name, position));
        c->pos = position;
        position += c->getSize();
    }
}

void make_bin(GameData &gameData, std::ostream &dbgout) {
    // if (gameData.nodes.count("start") == 0) {
    //     throw BuildError("Game lacks \"start\" node.");
    // }

    std::fstream out("game.bin", std::ios::out | std::ios::in | std::ios::binary | std::ios::trunc);
    if (!out) {
        throw BuildError("Could not open output file.");
    }

    // add space for header
    out.put('G');   out.put('R');
    out.put('P');   out.put('G');
    out.put(0);     out.put(0);
    out.put(1);     out.put(0);
    for (int i = 0; i < 24; ++i) {
        out.put(0);
    }

    std::unordered_map<std::string, unsigned> labels;
    std::uint32_t pos = 32;
    labels.insert(std::make_pair("true", 1));
    labels.insert(std::make_pair("false", 0));
    labels.insert(std::make_pair("stack", stackOperand));

    for (auto &c : gameData.constants) {
        labels.insert(std::make_pair(c.first, c.second));
    }

    std::uint8_t idByte = idString;
    for (auto &str : gameData.strings) {
        labels.insert(std::make_pair(str.second, pos));
        pos += str.first.size() + 2;
        out.write(reinterpret_cast<char*>(&idByte), 1);
        out.write(str.first.c_str(), str.first.size());
        out.put(0);
    }

    labels.insert(std::make_pair("__skill_table", pos));
    pos += 4;
    for (int i = 0; i < gameData.skills.size(); ++i) {
        labels.insert(std::make_pair(gameData.skills[i]->name, i));
        gameData.skills[i]->pos = pos;
        pos += sklSize;
    }

    doPositioning(labels, pos, gameData.sexes);
    doPositioning(labels, pos, gameData.species);
    doPositioning(labels, pos, gameData.items);
    doPositioning(labels, pos, gameData.characters);

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

            const std::string &cmdName = stmt->parts.front().text;
            const Command *cmd = getCommand(cmdName);
            if (!cmd) {
                throw BuildError(stmt->origin, "Unknown command " + cmdName);
            }

            if (stmt->parts.size() != (unsigned)cmd->argCount + 1) {
                throw BuildError(stmt->origin, "Wrong arg count.");
            }

            unsigned size = 0;
            if (cmdName == "label") {
                labels.insert(std::make_pair(mangleLabel(nodeName, stmt->parts.back().text), pos));
            } else {
                size = 1 + (stmt->parts.size() - 1) * 4;
            }
            pos += size;
        }
    }

    dbgout << "\n\nLABELS (" << labels.size() << "):\n" << std::hex << std::setfill('0');
    for (auto &label : labels) {
        dbgout << "0x" << std::setw(8) << label.second << ": " << label.first << '\n';
    }
    dbgout << std::dec;

    writeWord(out, gameData.skills.size());
    for (auto &skill : gameData.skills) {
        writeValue(out, skill->origin, labels, skill->statSkill);
        writeLabelValue(out, labels, skill->displayName);
        writeValue(out, skill->origin, labels, skill->defaultValue);
    }

    idByte = idSex;
    for (auto &sex : gameData.sexes) {
        out.write(reinterpret_cast<char*>(&idByte), 1);
        writeFlags(out, sex->origin, labels, sex->flags);
        writeLabelValue(out, labels, sex->displayName);
        writeLabelValue(out, labels, sex->subject);
        writeLabelValue(out, labels, sex->object);
        writeLabelValue(out, labels, sex->possess);
        writeLabelValue(out, labels, sex->adject);
        writeLabelValue(out, labels, sex->reflex);
    }

    idByte = idSpecies;
    for (auto &species : gameData.species) {
        out.write(reinterpret_cast<char*>(&idByte), 1);
        writeFlags(out, species->origin, labels, species->flags);
        writeLabelValue(out, labels, species->displayName);
    }

    idByte = idItem;
    for (auto &item : gameData.items) {
        out.write(reinterpret_cast<char*>(&idByte), 1);
        writeFlags(out, item->origin, labels, item->flags);
        writeLabelValue(out, labels, item->article);
        writeLabelValue(out, labels, item->singular);
        writeLabelValue(out, labels, item->plural);
        writeValue(out, item->origin, labels, item->onUse);
    }

    idByte = idCharacter;
    for (auto &who : gameData.characters) {
        out.write(reinterpret_cast<char*>(&idByte), 1);
        writeFlags(out, who->origin, labels, who->flags);
        writeLabelValue(out, labels, who->article);
        writeLabelValue(out, labels, who->displayName);
        writeValue(out, who->origin, labels, who->sex);
        writeValue(out, who->origin, labels, who->species);
        writeValue(out, who->origin, labels, who->faction);
        writeWord(out, who->skills.size());
        writeWord(out, who->gear.size());
        for (const auto &skill : who->skills) {
            writeLabelValue(out, labels, skill.first);
            writeValue(out, who->origin, labels, skill.second);
        }
        for (const auto &gear : who->gear) {
            writeLabelValue(out, labels, gear.first);
            writeLabelValue(out, labels, gear.second);
        }
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
            std::list<Value>::iterator cur = stmt->parts.begin();
            ++cur;
            while (cur != stmt->parts.end()) {
                uint32_t v = processValue(stmt->origin, labels, *cur, nodeName);
                out.write((const char *)&v, 4);
                ++cur;
            }
        }
    }

    std::uint32_t v;
    out.seekp(headerStartNode);
    writeLabelValue(out, labels, "start");
    writeLabelValue(out, labels, gameData.title);
    writeLabelValue(out, labels, gameData.byline);
    writeLabelValue(out, labels, gameData.version);
    writeLabelValue(out, labels, "__skill_table");

    time_t theTime = time(nullptr);
    struct tm *aTime = localtime(&theTime);
    v = (aTime->tm_year + 1900) * 10000;
    v += (aTime->tm_mon + 1) * 100;
    v += (aTime->tm_mday);
    out.write((const char *)&v, 4);
}