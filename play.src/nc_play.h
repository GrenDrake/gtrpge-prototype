#ifndef NC_PLAY_H
#define NC_PLAY_H

#include <string>
#include "play.h"

void doCharacter(Game &game);
void doInventory(Game &game);

void addToOutput(const std::string &text);

extern WINDOW *statusWindow;
extern PANEL *statusPanel;
extern WINDOW *mainWindow;
extern PANEL *mainPanel;
extern WINDOW *optionsWindow;
extern PANEL *optionsPanel;
extern WINDOW *subWindow;
extern PANEL *subPanel;

#endif
