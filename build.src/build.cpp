#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "errorlog.h"
#include "project.h"
#include "build.h"
#include "data.h"


BuildError::BuildError(const std::string &msg)
: std::runtime_error(msg), errorMessage("")
{
    strncpy(errorMessage, msg.c_str(), errorMessageLength-1);
}
BuildError::BuildError(const Origin &origin, const std::string &msg)
: std::runtime_error(msg), errorMessage("")
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
        throw BuildError(Origin(), "Could not open file "+file+".");
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

void showErrorLog(ErrorLog &log) {
    for (const ErrorLog::ErrorMessage &message : log.messages) {
        switch(message.type) {
            case ErrorLog::Error:
                std::cerr << "ERROR: ";
                break;
            case ErrorLog::Warning:
                std::cerr << "WARING: ";
                break;
            default:
                std::cerr << "UNKNOWN MESSAGE TYPE: ";
        }
        std::cerr << message.origin << ' ';
        std::cerr << message.message << '\n';
    }
}

int main(int argc, char *argv[]) {
    GameData gameData;
    ErrorLog log;
    Lexer lexer(log);

    if (argc != 2) {
        std::cerr << "USAGE: build <project-file>\n";
        return 1;
    }
    ProjectFile *project = load_project(argv[1]);

    if (project->sourceFiles.empty()) {
        std::cerr << "No source files provided!\n";
        return 1;
    }

    ObjectDef::setPropertyIdent("name", propName);
    ObjectDef::setPropertyIdent("article", propArticle);
    ObjectDef::setPropertyIdent("plural", propPlural);
    ObjectDef::setPropertyIdent("class", propClass);
    ObjectDef::setPropertyIdent("subject", propSubject);
    ObjectDef::setPropertyIdent("object", propObject);
    ObjectDef::setPropertyIdent("possessive", propPossessive);
    ObjectDef::setPropertyIdent("adjective", propAdjective);
    ObjectDef::setPropertyIdent("reflexive", propReflexive);
    ObjectDef::setPropertyIdent("cost-skill", propCostSkill);
    ObjectDef::setPropertyIdent("cost-amount", propCostAmount);
    ObjectDef::setPropertyIdent("peace-node", propPeaceNode);
    ObjectDef::setPropertyIdent("combat-node", propCombatNode);
    ObjectDef::setPropertyIdent("on-use", propOnUse);
    ObjectDef::setPropertyIdent("can-equip", propCanEquip);
    ObjectDef::setPropertyIdent("on-equip", propOnEquip);
    ObjectDef::setPropertyIdent("on-remove", propOnRemove);
    ObjectDef::setPropertyIdent("slot", propSlot);
    ObjectDef::setPropertyIdent("action-list", propActionList);
    ObjectDef::setPropertyIdent("skills", propSkills);
    ObjectDef::setPropertyIdent("faction", propFaction);
    ObjectDef::setPropertyIdent("gear", propGear);
    ObjectDef::setPropertyIdent("base-abilities", propBaseAbilities);
    ObjectDef::setPropertyIdent("extra-abilities", propExtraAbilities);
    ObjectDef::setPropertyIdent("description", propDescription);
    ObjectDef::setPropertyIdent("internal-name", propInternalName);
    ObjectDef::setPropertyIdent("sex", propSex);
    ObjectDef::setPropertyIdent("species", propSpecies);
    ObjectDef::setPropertyIdent("ai", propAi);
    ObjectDef::setPropertyIdent("target", propTarget);
    ObjectDef::setPropertyIdent("body", propBody);
    ObjectDef::setPropertyIdent("duration", propDuration);
    ObjectDef::setPropertyIdent("location", propLocation);
    ObjectDef::setPropertyIdent("ident", propIdent);

    try {
        std::vector<SymbolDef> symbols;

        Parser parser(gameData, symbols);
        for (const std::string &file : project->sourceFiles) {
            lexer.doFile(file);
            if (log.foundErrors) {
                showErrorLog(log);
                delete project;
                return 1;
            }
            parser.parseTokens(lexer.tokens.begin(), lexer.tokens.end());
        }

        bool hasStartSymbol = false;
        for (const auto &symbol : symbols) {
            if (symbol.name == "start") {
//                if (symbol.type != SymbolDef::Node) {
//                    std::cerr << symbol.origin << " Start node must be node.\n";
//                    return 1;
//                }
                hasStartSymbol = true;
            }
        }
        if (!hasStartSymbol) {
            std::cerr << "FATAL: No start node defined.\n";
            delete project;
            return 1;
        }
    } catch (BuildError &e) {
        std::cerr << e.what() << "\n";
        delete project;
        return 1;
    }

    try {
        for (std::shared_ptr<DataType> i : gameData.dataItems) {
            std::shared_ptr<SkillSet> cd = std::dynamic_pointer_cast<SkillSet>(i);
            if (cd) {
                for (unsigned i = 0; i < sklCount; ++i) {
                    if (cd->setDefaults) {
                        if (i >= gameData.skills.size()) {
                            cd->skills[i] = 0;
                        } else {
                            cd->skills[i] = gameData.skills[i]->defaultValue;
                        }
                    } else {
                        cd->skills[i] = 0;
                    }
                }

                for (auto &i : cd->skillMap) {
                    const auto &v = gameData.constants.find(i.first);
                    if (v != gameData.constants.end()) {
                        cd->skills[v->second.value] = i.second.value;
                    }
                }
                cd->skillMap.clear();
            }
        }

        make_bin(gameData, project->outputFile);
    } catch (BuildError &e) {
        std::cerr << e.what() << "\n";
    }


    /* ************************************************************************
     * DUMP GLOBAL SYMBOLS TO FILE                                            */
    std::ofstream out_defs("dbg_defs.txt");

    out_defs << ObjectDef::propertyNames.size() << " Object Properties\n";
    for (auto &propName : ObjectDef::propertyNames) {
        out_defs << "    " << propName.first << ": " << propName.second << '\n';
    }

    out_defs << "\nFOUND " << std::dec << gameData.dataItems.size() << " DATA ITEMS\n" << std::hex << std::uppercase;
    for (auto &item : gameData.dataItems) {
        out_defs << "    " << *item << '\n';
    }

    out_defs << "\nFOUND " << gameData.nodes.size() << " NODES\n";
    for (auto &node : gameData.nodes) {
        out_defs << "    " << std::setw(32) << node->name << ' ' << node->origin << '\n';
    }

    out_defs << "\nFOUND " << gameData.skills.size() << " SKILLS\n" << std::left;
    for (auto &skill : gameData.skills) {
        out_defs << std::left << "    " << std::setw(32);
        if (skill->name.empty()) {
            out_defs << "(unnamed)";
        } else {
            out_defs << skill->name;
        }
        out_defs << "   " << skill->origin << '\n';
    }

    out_defs << "\nFOUND " << gameData.constants.size() << " CONSTANTS:\n" << std::left;
    for (auto &constant : gameData.constants) {
        out_defs << "    " << std::setw(32) << constant.first << "   ";
        out_defs << std::setw(13) << constant.second;
        // << origin
        out_defs << '\n';
    }

    out_defs << "\nFOUND " << gameData.strings.size() << " STRINGS:\n";
    for (auto &str : gameData.strings) {
        out_defs << "    " << std::setw(8) << str.second << "  ";
        std::stringstream ss;
        ss << "~" << formatForDump(str.first, 35) << "~";
        out_defs << std::setw(37) << ss.str();
        // << origin
        out_defs << " \n";
    }

    out_defs.close();

    /* ************************************************************************
     * DUMP DATA ITEMS TO FILE                                                */
    std::ofstream out("dbg_nodes.txt");

    out << "FOUND " << gameData.nodes.size() << " NODES\n";
    for (auto &node : gameData.nodes) {
        out << '\n' << node->name << '\n';
        for (auto &stmt : node->block->statements) {
            out << "   ";
            for (auto &text : stmt->parts) {
                out << ' ' << text;
            }
            out << ";\n";
        }
    }

    /* ********************************************************************** */
    delete project;
    return 0;
}