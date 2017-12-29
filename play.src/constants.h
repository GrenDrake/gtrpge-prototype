#ifndef CONSTANTS_H
#define CONSTANTS_H

const int headerStartNode   = 0x08;
const int headerTitle       = 0x0C;
const int headerByline      = 0x10;
const int headerVersion     = 0x14;
const int headerSkillTable  = 0x18;
const int headerBuildNumber = 0x22;

const int idString          = 0xFF;
const int idNode            = 0xFE;
const int idItem            = 0xFD;
const int idSex             = 0xFC;
const int idSpecies         = 0xFB;
const int idCharacter       = 0xFA;

const int itmFlags          = 1;
const int itmArticle        = 5;
const int itmSingular       = 9;
const int itmPlural         = 13;
const int itmOnUse          = 17;
const int itmSize           = 21;

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
const int sklCount          = 32;

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
const int chrGearList       = 25+(sklCount);
const int cgSlot            = 0;
const int cgItem            = 4;
const int cgSize            = 8;

const int opEnd             = 0x00;
const int opDoNode          = 0x01;
const int opSetLocation     = 0x02;
const int opHasFlag         = 0x03;
const int opPush            = 0x0E;
const int opPop             = 0x0F;
const int opAddOption       = 0x10;
const int opAddOptionXtra   = 0x11;
const int opAddContinue     = 0x12;
const int opAddReturn       = 0x13;
const int opSay             = 0x20;
const int opSayNumber       = 0x21;
const int opJump            = 0x30;
const int opJumpTrue        = 0x31;
const int opJumpFalse       = 0x32;
const int opJumpEq          = 0x33;
const int opJumpNeq         = 0x34;
const int opJumpLt          = 0x35;
const int opJumpLte         = 0x36;
const int opJumpGt          = 0x37;
const int opJumpGte         = 0x38;
const int opJumpItemQty     = 0x39;
const int opStore           = 0x40;
const int opFetch           = 0x41;
const int opAddItems        = 0x50;
const int opRemoveItems     = 0x51;
const int opItemQty         = 0x52;
const int opIncrement       = 0x60;
const int opDecrement       = 0x61;
const int opAddToList       = 0x70;
const int opRemoveFromList  = 0x71;
const int opIsInList        = 0x72;
const int opRandomFromList  = 0x73;
const int opCreateList      = 0x74;
const int opAddToListChance = 0x75;
const int opMakeCharacter   = 0x80;
const int opGetSex          = 0x81;
const int opSetSex          = 0x82;
const int opGetSpecies      = 0x83;
const int opSetSpecies      = 0x84;
const int opGetSkill        = 0x85;
const int opAdjSkill        = 0x86;
const int opGetSkillCur     = 0x87;
const int opAdjSkillCur     = 0x88;

const unsigned xtraValue        = 0xFFFFFFFE;

const int operandNone           = 0;
const int operandImmediate      = 1;
const int operandStorage        = 2;
const int operandStack          = 3;

#endif