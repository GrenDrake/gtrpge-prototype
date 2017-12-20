#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
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

std::string mangleLabel(const std::string &nodeName, const std::string &original) {
    std::string labelName = "__n_"+nodeName+"_"+original;
    return labelName;
}

void make_bin(GameData &gameData, std::ostream &dbgout) {
    if (gameData.nodes.count("start") == 0) {
        throw BuildError("Game lacks \"start\" node.");
    }

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

    std::uint8_t idByte = idString;
    for (auto &str : gameData.strings) {
        labels.insert(std::make_pair(str.second, pos));
        pos += str.first.size() + 2;
        out.write(reinterpret_cast<char*>(&idByte), 1);
        out.write(str.first.c_str(), str.first.size());
        out.put(0);
    }

    for (auto &node : gameData.nodes) {
        const std::string &nodeName = node.second->name;
        labels.insert(std::make_pair(nodeName, pos));
        ++pos;
        for (auto &stmt : node.second->block->statements) {
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


    idByte = idNode;
    for (auto &node : gameData.nodes) {
        const std::string &nodeName = node.second->name;
        out.write(reinterpret_cast<char*>(&idByte), 1);
        for (auto &stmt : node.second->block->statements) {
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
                int v = 0;
                switch(cur->type) {
                    case Value::Integer:
                        v = cur->value;
                        break;
                    case Value::Identifier:
                        if (labels.count(cur->text) == 0) {
                            std::string mangled = mangleLabel(nodeName, cur->text);
                            if (labels.count(mangled)) {
                                v = labels[mangled];
                            } else {
                                std::cerr << "WARNING: " << stmt->origin << " Unknown symbol " << cur->text << '\n';
                            }
                        } else {
                            v = labels[cur->text];
                        }
                        break;
                }
                out.write((const char *)&v, 4);
                ++cur;
            }
        }
    }

    std::uint32_t v;
    out.seekp(headerStartNode);
    v = labels["start"];
    out.write((const char *)&v, 4);
    v = labels[gameData.title];
    out.write((const char *)&v, 4);
    v = labels[gameData.byline];
    out.write((const char *)&v, 4);
    v = labels[gameData.version];
    out.write((const char *)&v, 4);

    time_t theTime = time(nullptr);
    struct tm *aTime = localtime(&theTime);
    v = (aTime->tm_year + 1900) * 10000;
    v += (aTime->tm_mon + 1) * 100;
    v += (aTime->tm_mday);
    out.write((const char *)&v, 4);
}