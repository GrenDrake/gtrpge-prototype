#ifndef NC_PLAY_H
#define NC_PLAY_H

#include <string>
#include "play.h"

void drawStatus(Game &game);
void doCharacter(Game &game);
void doInventory(Game &game);

void addToOutput(const std::string &text);
bool getYesNo(const std::string &prompt, bool defaultAnswer);
std::string getString(const std::string &prompt, unsigned maxlen, const std::string &initialText = "");
void showMessageBox(const std::string &message);

const int colorOptions = 1;
const int colorMain = 2;
const int colorDialog = 3;
const int colorStatus = 4;

#endif
