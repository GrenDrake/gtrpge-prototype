#include <cctype>
#include <ncurses.h>
#include <sstream>

#include "nc_play.h"

const int modeStats = 0;
const int modeGear  = 1;

void doCharacter(Game &game) {
    if (game.party.empty()) return;
    int maxX = 0, maxY = 0;

    unsigned curParty = 0;
    int curChar = game.party[curParty];
    int mode = modeStats;
    unsigned selection = 0;
    uint32_t curGear = 0;

    while(true) {
        getmaxyx(stdscr, maxY, maxX);
        bkgdset(A_NORMAL | COLOR_PAIR(colorMain));
        clear();
        drawStatus(game);

        std::stringstream helpLine;
        helpLine << "<G>ear   <S>tats   <P>rev Char   <N>ext Char   <Z> Close";
        std::stringstream infoLine;
        Character *c = game.getCharacter(curChar);
        infoLine << toTitleCase(game.getNameOf(curChar)) << " (" << toTitleCase(game.getNameOf(c->sex)) << ' ';
        infoLine << toTitleCase(game.getNameOf(c->species)) << ")";
        switch(mode) {
            case modeStats:
                infoLine << "   - Statisitics";
                break;
            case modeGear:
                helpLine << "   <U>nequip";
                infoLine << "   - Equipped Gear";
                break;
            default:
                infoLine << "   - Unknown Display Mode";
        }
        bkgdset(A_NORMAL | COLOR_PAIR(colorStatus));
        move(maxY-1, 0);
        clrtoeol();
        mvprintw(maxY - 1, 1, helpLine.str().c_str());
        bkgdset(A_NORMAL | COLOR_PAIR(colorMain));
        mvprintw(2, 0, infoLine.str().c_str());

        if (mode == modeStats) {
            int counter = 0;
            std::uint32_t skillTable = game.readWord(headerSkillTable);
            for (int i = 0; i < sklCount; ++i) {
                std::uint32_t nameAddr = game.readWord(skillTable + i*sklSize + sklName);
                if (!nameAddr) continue;

                std::stringstream ss;
                const char *name = game.getString(nameAddr);
                ss << toTitleCase(name) << ": ";
                if (game.testSkillFlags(i, sklVariable)) {
                    ss << game.getSkillCur(curChar, i);
                    ss << '/';
                }
                ss << game.getSkillMax(curChar, i);

                int x = 0, y = 0;
                if (counter < 8) {
                    x = 2;
                    y = 4+counter;
                } else {
                    x = (COLS-8) / 2;
                    y = counter - 4;
                }
                ++counter;
                mvprintw(y, x, ss.str().c_str());
            }

        } else if (mode == modeGear) {
            if (c->gear.empty()) {
                curGear = 0;
                mvprintw(3, 3, "Nothing equipped.");
            } else {
                unsigned counter = 0;
                for (auto i : c->gear) {
                    std::stringstream ss;
                    ss << (counter+1) << ") " << toTitleCase(game.getString(i.first)) << ": " << game.getNameOf(i.second);
                    int y = 4+counter;
                    if (selection == counter) {
                        bkgdset(A_REVERSE);
                        curGear = i.first;
                    }
                    move(y, 0);
                    clrtoeol();
                    mvprintw(y, 1, ss.str().c_str());
                    // mvprintw(y, 40, "Equipment Stats");
                    bkgdset(A_NORMAL);
                    ++counter;
                }
            }
        }

        refresh();

        int key = toupper(getch());
        switch(key) {
            case 'Z':
                return;
            case 'S':
                mode = modeStats;
                break;
            case 'G':
                if (mode != modeGear) {
                    mode = modeGear;
                    selection = 0;
                }
                break;
            case KEY_NPAGE:
            case 'N':
                ++curParty;
                if (curParty >= game.party.size()) {
                    curParty = 0;
                }
                curChar = game.party[curParty];
                break;
            case KEY_PPAGE:
            case 'P':
                if (curParty == 0) {
                    curParty = game.party.size() - 1;
                } else {
                    --curParty;
                }
                curChar = game.party[curParty];
                break;
            case 'U':
                if (mode == modeGear) {
                    game.unequipItem(curChar, curGear);
                    if (selection > c->gear.size() - 1) {
                        selection = c->gear.size() - 1;
                    }
                }
                break;
            case KEY_DOWN:
                if (mode == modeGear && selection < c->gear.size() - 1) {
                    ++selection;
                }
                break;
            case KEY_UP:
                if (selection > 0) {
                    --selection;
                }
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (mode == modeGear) {
                    unsigned newSel = key - '1';
                    if (newSel < c->gear.size()) {
                        selection = newSel;
                    }
                }
        }
    }
}