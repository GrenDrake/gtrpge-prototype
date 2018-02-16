#ifndef CONSTANTS_H
#define CONSTANTS_H

// GameFile Header Structure
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

// Data Type IDs
const int idString          = 0xFF;
const int idNode            = 0xFE;
const int idList            = 0xF9;
const int idSkillSet        = 0xF8;
const int idObject          = 0xF6;

// SkillDef Structure
const int sklBaseSkill      = 0;
const int sklName           = 4;
const int sklFlags          = 8;
const int sklDefault        = 12;
const int sklRecovery       = 16;
const int sklSize           = 20;
const int sklCount          = 20;
const int sklSetSize        = 1 + sklCount * 2;

const int sklX5Multiplier   = 50;

// SkillDef Flags
const int sklVariable       = 0x01; // variable stat (like health, energy, mana, corruption, etc.)
const int sklKOZero         = 0x02; // KO character if reaches 0
const int sklKOFull         = 0x04; // KO character is becomes full
const int sklResetOnRest    = 0x08; // current skill value is reset when character rests
const int sklX5             = 0x10; // multiply (max) value of skill by 5 for final value
const int sklOnTracker      = 0x20; // show on combat tracker

// Predefined Properties
const int propName              = 1;
const int propArticle           = 2;
const int propPlural            = 3;
const int propClass             = 4;
const int propSubject           = 5;
const int propObject            = 6;
const int propAdjective         = 7;
const int propPossessive        = 8;
const int propReflexive         = 9;
const int propCostSkill         = 10;
const int propCostAmount        = 11;
const int propCombatNode        = 12;
const int propPeaceNode         = 13;
const int propOnUse             = 14;
const int propCanEquip          = 15;
const int propOnEquip           = 16;
const int propOnRemove          = 17;
const int propSlot              = 18;
const int propActionList        = 19;
const int propSkills            = 20;
const int propFaction           = 21;
const int propGear              = 22;
const int propBaseAbilities     = 23;
const int propExtraAbilities    = 24;
const int propDescription       = 25;
const int propInternalName      = 26;
const int propSex               = 27;
const int propSpecies           = 28;
const int propAi                = 29;
const int propTarget            = 30;
const int propBody              = 31;
const int propLocation          = 32;
const int propDuration          = 33;

const int propFirstCustom       = 256;

// ObjectDef classes
const int ocSpecies         = 1;
const int ocSex             = 2;
const int ocAction          = 3;
const int ocItem            = 4;
const int ocCharacter       = 5;
const int ocScene           = 6;

// DamageType Count
const int damageTypeCount   = 8;

// Ability combat targets
const int targetNone        = 0;
const int targetAlly        = 1;
const int targetEnemy       = 2;
const int targetAny         = 3;

// Opcode/command numbers
const int opEnd             = 0x00;
const int opDoNode          = 0x01;
const int opSetLocation     = 0x02;
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
const int opSayPronoun      = 0x24;
const int opSayPronounUF    = 0x25;
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
const int opCombatant       = 0xB2;
const int opGetProperty     = 0xC0;
const int opRandomOfFaction = 0xC1;
const int opRandomNotFaction= 0xC2;
const int opStackSwap       = 0xC3;
const int opStackDup        = 0xC4;
const int opStackCount      = 0xC5;
const int opIsKOed          = 0xC6;
const int opHasProperty     = 0xC7;
const int opPartySize       = 0xC8;
const int opPartyIsKOed     = 0xC9;
const int opDoRest          = 0xCA;
const int opCombatStatus    = 0xCB;
const int opPartyAt         = 0xCC;
const int opGetEquip        = 0xCD;
const int opSetEquip        = 0xCE;

const int optionNameContinue    = 1;
const int optionNameCancel      = 2;

// Temporary Storage Indexes
const unsigned storageFirstTemp = 0xFFFFFFFF;
const unsigned storageTempCount = 10;

#endif