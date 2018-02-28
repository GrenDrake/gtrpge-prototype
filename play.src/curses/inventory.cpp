#include <ncurses.h>
#include <sstream>

#include "play.h"

const int invPerPage = 9;

void doInventory(Game &game) {
    int maxX = 0, maxY = 0;

    if (game.inventory.empty()) {
        maxY = getmaxy(stdscr);
        bkgdset(A_NORMAL | COLOR_PAIR(colorMain));
        clear();
        drawStatus(game);
        bkgdset(A_NORMAL | COLOR_PAIR(colorMain));
        mvprintw(1, 0, "You are carrying:");
        mvprintw(3, 0, "Nothing");
        bkgdset(A_NORMAL | COLOR_PAIR(colorStatus));
        move(maxY-1, 0);
        clrtoeol();
        mvprintw(maxY-1, 0, "Press any key to return");
        refresh();
        getch();
        return;
    }


    unsigned curItem = 0;
    while (true) {
        getmaxyx(stdscr, maxY, maxX);
        bkgdset(A_NORMAL | COLOR_PAIR(colorMain));
        clear();
        drawStatus(game);
        bkgdset(A_NORMAL | COLOR_PAIR(colorStatus));
        move(maxY-1, 0);
        clrtoeol();
        mvprintw(maxY-1, 0, "<SPACE> Use   <E>quip   <Z> Cancel");
        bkgdset(A_NORMAL | COLOR_PAIR(colorMain));
        mvprintw(1, 0, "You are carrying:");

        unsigned curPage = curItem / invPerPage;
        if (curPage > 0) {
            attrset(A_REVERSE);
            mvprintw(2, maxX-10, "(MORE)");
            attrset(A_NORMAL);
        }
        if (curPage + invPerPage < game.inventory.size()) {
            attrset(A_REVERSE);
            mvprintw(12, maxX-10, "(MORE)");
            attrset(A_NORMAL);
        }

        for (unsigned i = 0; i < invPerPage && i + curPage * invPerPage < game.inventory.size(); ++i) {
            auto item = game.inventory[i + curPage * invPerPage];

            std::stringstream itemName, itemTraits;

            if (item.qty == 1) {
                itemName << toUpperFirst(game.getNameOf(game.getObjectProperty(item.itemIdent, propArticle)));
                itemName << game.getNameOf(game.getObjectProperty(item.itemIdent, propName));
            } else {
                itemName << item.qty << ' ';
                itemName << game.getNameOf(game.getObjectProperty(item.itemIdent, propPlural));
            }

            if (game.getObjectProperty(item.itemIdent, propOnUse)) {
                itemTraits << "Usable  ";
            }

            if (game.getObjectProperty(item.itemIdent, propSlot)) {
                itemTraits << "Equip as: " << toTitleCase(game.getNameOf(game.getObjectProperty(item.itemIdent, propSlot)));
            }

            if (curItem == i + curPage * invPerPage) {
                bkgdset(A_REVERSE | COLOR_PAIR(colorMain));
            }
            move(i + 3, 0);
            clrtoeol();
            mvprintw(i + 3, 1, "%c)", '1' + i);
            mvprintw(i + 3, 5, itemName.str().c_str());
            mvprintw(i + 3, maxX/2, itemTraits.str().c_str());
            bkgdset(A_NORMAL | COLOR_PAIR(colorMain));

        }

        std::uint32_t descString = game.getObjectProperty(game.inventory[curItem].itemIdent, propDescription);
        if (descString) {
            auto i = wrapString(game.getNameOf(descString), COLS-12);
            mvprintw(13, 3, i[0].c_str());
            if (i.size() > 1) {
                mvprintw(14, 3, i[1].c_str());
            }
        }

        refresh();
        int key = toupper(getch());

        if (key == 'Z') {
            break;
        } else if (key == KEY_ENTER || key == 'U' || key == ' ' || key == '\n' || key == '\r') {
            if (!game.getObjectProperty(game.inventory[curItem].itemIdent, propOnUse)) {
                continue;
            }
            game.useItem(curItem);
            addToOutput(game.getOutput());
            break;
        } else if (key == 'E') {
            std::uint32_t slot = game.getObjectProperty(game.inventory[curItem].itemIdent, propSlot);
            if (!slot) {
                continue;
            }
            game.equipItem(game.party[0], curItem);
            continue;
        } else if (key == 'N' || key == KEY_DOWN) {
            if (curItem < game.inventory.size() - 1) {
                ++curItem;
            }
        } else if (key == 'P' || key == KEY_UP) {
            if (curItem > 0) {
                --curItem;
            }
        } else if (key == KEY_NPAGE) {
            curItem += invPerPage;
            if (curItem >= game.inventory.size()) {
                curItem = game.inventory.size() - 1;
            }
        } else if (key == KEY_PPAGE) {
            if (curItem > invPerPage) {
                curItem -= invPerPage;
            } else {
                curItem = 0;
            }
        } else if (key >= '1' && key <= '9' && key < '1' + static_cast<int>(game.inventory.size())) {
            curItem = key - '1';
        }
    }
}
