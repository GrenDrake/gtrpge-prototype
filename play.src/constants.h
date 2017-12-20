#ifndef CONSTANTS_H
#define CONSTANTS_H

const int headerStartNode   = 0x08;

const int idString          = 0xFF;
const int idNode            = 0xFE;

const int opEnd             = 0x00;
const int opDoNode          = 0x01;
const int opSetLocation     = 0x02;
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

const int stackOperand      = 0xFFFFFFFF;
#endif