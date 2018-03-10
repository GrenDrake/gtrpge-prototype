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

// Property Type IDs
const int pidInteger        = 0x7F;
const int pidReference      = 0x7E;
const int pidUndefined      = 0x7D;

// SkillDef Structure
const int sklBaseSkill      = 0;
const int sklName           = 4;
const int sklFlags          = 8;
const int sklDefault        = 12;
const int sklRecovery       = 16;
const int sklSize           = 20;
const int sklCount          = 20;
const int sklX5Multiplier   = 50;

// Object Property Structure
const int objPropId         = 0;
const int objPropType       = 2;
const int objPropValue      = 4;
const int objPropSize       = 8;

// Map structure
const int gmapCount         = 1;
const int gmapHeader        = 5;
const int gmapEntrySize     = 8;

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
const int propIdent             = 34;

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
enum Opcodes {
    opEnd             = 0x00,
    opPush            = 0x01,
    opPop             = 0x02,
    opCallNode        = 0x03,
    opSetLocation     = 0x04,
    opStartGame       = 0x05,
    opAddTime         = 0x06,
    opAddOption       = 0x07,
    opAddOptionXtra   = 0x08,
    opAddContinue     = 0x09,
    opAddReturn       = 0x0A,
    opSay             = 0x0B,
    opSayNumber       = 0x0C,
    opSayUF           = 0x0D,
    opSayTC           = 0x0E,
    opSayPronoun      = 0x0F,
    opSayPronounUF    = 0x10,
    opJump            = 0x11,
    opJumpTrue        = 0x12,
    opJumpFalse       = 0x13,
    opJumpEq          = 0x14,
    opJumpNeq         = 0x15,
    opJumpLt          = 0x16,
    opJumpLte         = 0x17,
    opJumpGt          = 0x18,
    opJumpGte         = 0x19,
    opStore           = 0x1A,
    opFetch           = 0x1B,
    opAddItems        = 0x1C,
    opRemoveItems     = 0x1D,
    opItemQty         = 0x1E,
    opAddToList       = 0x1F,
    opRemoveFromList  = 0x20,
    opIsInList        = 0x21,
    opRandomFromList  = 0x22,
    opCreateList      = 0x23,
    opAddToListChance = 0x24,
    opResetCharacter  = 0x25,
    opGetSex          = 0x26,
    opSetSex          = 0x27,
    opGetSpecies      = 0x28,
    opSetSpecies      = 0x29,
    opGetSkill        = 0x30,
    opAdjSkill        = 0x31,
    opGetSkillCur     = 0x32,
    opAdjSkillCur     = 0x33,
    opSkillCheck      = 0x34,
    opDoDamage        = 0x35,
    opAdd             = 0x36,
    opSubtract        = 0x37,
    opMultiply        = 0x38,
    opDivide          = 0x39,
    opModulo          = 0x3A,
    opPower           = 0x3B,
    opIncrement       = 0x3C,
    opDecrement       = 0x3D,
    opAddToParty      = 0x3E,
    opIsInParty       = 0x3F,
    opRemoveFromParty = 0x40,
    opResetCombat     = 0x41,
    opAddToCombat     = 0x42,
    opCombatant       = 0x43,
    opGetProperty     = 0x44,
    opRandomOfFaction = 0x45,
    opRandomNotFaction= 0x46,
    opStackSwap       = 0x47,
    opStackDup        = 0x48,
    opStackCount      = 0x49,
    opIsKOed          = 0x4A,
    opHasProperty     = 0x4B,
    opPartySize       = 0x4C,
    opPartyIsKOed     = 0x4D,
    opDoRest          = 0x4E,
    opCombatStatus    = 0x4F,
    opPartyAt         = 0x50,
    opGetEquip        = 0x51,
    opSetEquip        = 0x52,
    opRandom          = 0x53,
};

const int optionNameContinue    = 1;
const int optionNameCancel      = 2;
const int optionDoNothing       = 3;

// Temporary Storage Indexes
const unsigned storageFirstTemp = 0xFFFFFFFF;
const unsigned storageTempCount = 10;

#endif