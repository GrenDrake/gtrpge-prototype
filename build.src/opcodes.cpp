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
    { "say-uf",          opSayUF,           0 },
    { "say-tc",          opSayTC,           0 },
    { "say-pronoun",     opSayPronoun,      0 },
    { "say-pronoun-uf",  opSayPronounUF,    0 },
    { "say-number",      opSayNumber,       0 },
    { "jump",            opJump,            0 },
    { "jump-true",       opJumpTrue,        0 },
    { "jump-false",      opJumpFalse,       0 },
    { "jump-eq",         opJumpEq,          0 },
    { "jump-neq",        opJumpNeq,         0 },
    { "jump-lt",         opJumpLt,          0 },
    { "jump-lte",        opJumpLte,         0 },
    { "jump-gt",         opJumpGt,          0 },
    { "jump-gte",        opJumpGte,         0 },
    { "store",           opStore,           0 },
    { "fetch",           opFetch,           0 },
    { "add-items",       opAddItems,        0 },
    { "remove-items",    opRemoveItems,     0 },
    { "item-qty",        opItemQty,         0 },
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
    { "add-to-party",       opAddToParty,       0 },
    { "is-in-party",        opIsInParty,        0 },
    { "remove-from-party",  opRemoveFromParty,  0 },
    { "reset-combat",       opResetCombat,      0 },
    { "add-to-combat",      opAddToCombat,      0 },
    { "get-property",       opGetProperty,      0 },
    { "combatant",          opCombatant,        0 },
    { "random-of-faction",  opRandomOfFaction,  0 },
    { "random-not-faction", opRandomNotFaction, 0 },
};

const Command* getCommand(const std::string name) {
    for (Command &cmd : commands) {
        if (name == cmd.text) {
            return &cmd;
        }
    }
    return nullptr;
}