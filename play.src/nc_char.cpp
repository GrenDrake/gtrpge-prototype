#include <ncurses.h>
#include <panel.h>
#include <sstream>

#include "nc_play.h"

void doCharacter(Game &game) {
    if (game.party.empty()) return;

    wclear(subWindow);
    show_panel(subPanel);
    box(subWindow, 0, 0);

    int curParty = 0;
    int curChar = game.party[curParty];

    mvwprintw(subWindow, 16, COLS-3-28-5, "Press any key to close");

    std::stringstream infoLine;
    Character *c = game.getCharacter(curChar);
    infoLine << toTitleCase(game.getNameOf(curChar)) << " (" << toTitleCase(game.getNameOf(c->sex)) << ' ';
    infoLine << toTitleCase(game.getNameOf(c->species)) << ")";
    mvwprintw(subWindow, 1, 2, infoLine.str().c_str());

    int counter = 0;
    std::uint32_t skillTable = game.readWord(headerSkillTable);
    for (int i = 0; i < sklCount; ++i) {
        std::uint32_t nameAddr = game.readWord(skillTable + i*sklSize + sklName);
        if (!nameAddr) continue;

        std::stringstream ss;
        const char *name = game.getString(nameAddr);
        ss << i << ") " << toTitleCase(name) << ": ";
        if (game.testSkillFlags(i, sklVariable)) {
            ss << game.getSkillCur(curChar, i);
            ss << '/';
        }
        ss << game.getSkillMax(curChar, i);
        ss << "\n";

        int x = 0, y = 0;
        if (counter < 8) {
            x = 2;
            y = 3+counter;
        } else {
            x = (COLS-6) / 2;
            y = counter - 5;
        }
        ++counter;
        mvwprintw(subWindow, y, x, ss.str().c_str());
    }

    update_panels();
    doupdate();

    getch();

    hide_panel(subPanel);
    update_panels();
    doupdate();
}