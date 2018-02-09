#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <map>

#include "build.h"
#include "../play.src/constants.h"

Command commands[] = {
    { "label",           -1,                1 },
    { "end",             opEnd,             0 },
    { "do-node",         opDoNode,          0 },
    { "start-game",      opStartGame,       0 },
    { "add-time",        opAddTime,         0 },
    { "push",            opPush,            1 },
    { "pop",             opPop,             0 },
    { "set-location",    opSetLocation,     0 },
    { "add-option",      opAddOption,       0 },
    { "add-option-xtra", opAddOptionXtra,   0 },
    { "add-continue",    opAddContinue,     0 },
    { "add-return",      opAddReturn,       0 },
    { "say",             opSay,             0 },
    { "say-uf",          opSayUF,           1 },
    { "say-tc",          opSayTC,           1 },
    { "say-pronoun",     opSayPronoun,      2 },
    { "say-pronoun-uf",  opSayPronounUF,    2 },
    { "say-number",      opSayNumber,       1 },
    { "jump",            opJump,            0 },
    { "jump-true",       opJumpTrue,        0 },
    { "jump-false",      opJumpFalse,       0 },
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
    { "add-to-list",        opAddToList,        2 },
    { "is-in-list",         opIsInList,         2 },
    { "remove-from-list",   opRemoveFromList,   2 },
    { "random-from-list",   opRandomFromList,   1 },
    { "create-list",        opCreateList,       1 },
    { "add-to-list-chance", opAddToListChance,  3 },
    { "reset-character",    opResetCharacter,   1 },
    { "get-sex",            opGetSex,           1 },
    { "set-sex",            opSetSex,           2 },
    { "get-species",        opGetSpecies,       1 },
    { "set-species",        opSetSpecies,       2 },
    { "get-skill",          opGetSkill,         2 },
    { "adj-skill",          opAdjSkill,         3 },
    { "get-skill-cur",      opGetSkillCur,      2 },
    { "adj-skill-cur",      opAdjSkillCur,      3 },
    { "skill-check",        opSkillCheck,       4 },
    { "do-damage",          opDoDamage,         4 },
    { "add",                opAdd,              0 },
    { "subtract",           opSubtract,         0 },
    { "multiply",           opMultiply,         0 },
    { "divide",             opDivide,           0 },
    { "modulo",             opModulo,           0 },
    { "power",              opPower,            0 },
    { "increment",          opIncrement,        0 },
    { "decrement",          opDecrement,        0 },
    { "add-to-party",       opAddToParty,       1 },
    { "is-in-party",        opIsInParty,        1 },
    { "remove-from-party",  opRemoveFromParty,  1 },
    { "reset-combat",       opResetCombat,      0 },
    { "add-to-combat",      opAddToCombat,      1 },
    { "get-property",       opGetProperty,      2 },
    { "combatant",          opCombatant,        1 },
    { "random-of-faction",  opRandomOfFaction,  1 },
    { "random-not-faction", opRandomNotFaction, 1 },
};

const Command* getCommand(const std::string name) {
    for (Command &cmd : commands) {
        if (name == cmd.text) {
            return &cmd;
        }
    }
    return nullptr;
}