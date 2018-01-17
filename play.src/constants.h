#ifndef CONSTANTS_H
#define CONSTANTS_H

const int headerFileID      = 0x00;
const int headerFileVersion = 0x04;
const int headerStartNode   = 0x08;
const int headerTitle       = 0x0C;
const int headerByline      = 0x10;
const int headerVersion     = 0x14;
const int headerSkillTable  = 0x18;
const int headerDamageTypes = 0x1C;
const int headerWeaponSlot  = 0x20;
const int headerBuildNumber = 0x24;
const int headerChecksum    = 0x28;
const int headerSize        = 64;

const int idString          = 0xFF;
const int idNode            = 0xFE;
const int idItem            = 0xFD;
const int idSex             = 0xFC;
const int idSpecies         = 0xFB;
const int idCharacter       = 0xFA;
const int idList            = 0xF9;
const int idSkillSet        = 0xF8;
const int idAction          = 0xF7;

const int itmFlags          = 1;
const int itmArticle        = 5;
const int itmSingular       = 9;
const int itmPlural         = 13;
const int itmDescription    = 17;
const int itmOnUse          = 21;
const int itmCanEquip       = 25;
const int itmOnEquip        = 29;
const int itmOnRemove       = 33;
const int itmSlot           = 37;
const int itmActionList     = 41;
const int itmSkillSet       = 45;
const int itmSize           = 49;

const int sexFlags          = 1;
const int sexName           = 5;
const int sexSubject        = 9;
const int sexObject         = 13;
const int sexPossessive     = 17;
const int sexAdjective      = 21;
const int sexReflexive      = 25;
const int sexSize           = 29;

const int spcFlags          = 1;
const int spcName           = 5;
const int spcSize           = 9;

const int sklBaseSkill      = 0;
const int sklName           = 4;
const int sklFlags          = 8;
const int sklSize           = 12;
const int sklCount          = 20;
const int sklSetSize        = 1 + sklCount * 2;

const int sklX5Multiplier   = 50;

const int sklVariable       = 0x01; // variable stat (like health, energy, mana, corruption, etc.)
const int sklKOZero         = 0x02; // KO character if reaches 0
const int sklKOFull         = 0x04; // KO character is becomes full
const int sklResetOnRest    = 0x08; // current skill value is reset when character rests
const int sklX5             = 0x10; // multiply (max) value of skill by 5 for final value

const int chrFlags          = 1;
const int chrArticle        = 5;
const int chrName           = 9;
const int chrSex            = 13;
const int chrSpecies        = 17;
const int chrFaction        = 21;
const int chrSkillDefaults  = 25;
const int chrGearList       = 29;
const int chrBaseAbilities  = 33;
const int chrExtraAbilities = 37;
const int chrSize           = 41;

const int actSkill          = 1;
const int actCost           = 5;
const int actName           = 9;
const int actCombatNode     = 13;
const int actPeaceNode      = 17;
const int actSize           = 21;

const int damageTypeCount   = 8;
const int opEnd             = 0x00;
const int opDoNode          = 0x01;
const int opSetLocation     = 0x02;
const int opHasFlag         = 0x03;
const int opStartGame       = 0x04;
const int opAddTime         = 0x05;
const int opPush            = 0x0E;
const int opPop             = 0x0F;
const int opAddOption       = 0x10;
const int opAddOptionXtra   = 0x11;
const int opAddContinue     = 0x12;
const int opAddReturn       = 0x13;
const int opSay             = 0x20;
const int opSayNumber       = 0x21;
const int opSayUF           = 0x22;
const int opSayTC           = 0x23;
const int opJump            = 0x30;
const int opJumpTrue        = 0x31;
const int opJumpFalse       = 0x32;
const int opJumpEq          = 0x33;
const int opJumpNeq         = 0x34;
const int opJumpLt          = 0x35;
const int opJumpLte         = 0x36;
const int opJumpGt          = 0x37;
const int opJumpGte         = 0x38;
const int opStore           = 0x40;
const int opFetch           = 0x41;
const int opAddItems        = 0x50;
const int opRemoveItems     = 0x51;
const int opItemQty         = 0x52;
const int opAddToList       = 0x70;
const int opRemoveFromList  = 0x71;
const int opIsInList        = 0x72;
const int opRandomFromList  = 0x73;
const int opCreateList      = 0x74;
const int opAddToListChance = 0x75;
const int opResetCharacter  = 0x80;
const int opGetSex          = 0x81;
const int opSetSex          = 0x82;
const int opGetSpecies      = 0x83;
const int opSetSpecies      = 0x84;
const int opGetSkill        = 0x85;
const int opAdjSkill        = 0x86;
const int opGetSkillCur     = 0x87;
const int opAdjSkillCur     = 0x88;
const int opSkillCheck      = 0x89;
const int opDoDamage        = 0x8A;
const int opAdd             = 0x90;
const int opSubtract        = 0x91;
const int opMultiply        = 0x92;
const int opDivide          = 0x93;
const int opModulo          = 0x94;
const int opPower           = 0x95;
const int opIncrement       = 0x96;
const int opDecrement       = 0x97;
const int opAddToParty      = 0xA0;
const int opIsInParty       = 0xA1;
const int opRemoveFromParty = 0xA2;
const int opResetCombat     = 0xB0;
const int opAddToCombat     = 0xB1;

const int operandNone           = 0;
const int operandImmediate      = 1;
const int operandStorage        = 2;
const int operandStack          = 3;

const unsigned storageFirstTemp = 0xFFFFFFFF;
const unsigned storageTempCount = 10;

#endif