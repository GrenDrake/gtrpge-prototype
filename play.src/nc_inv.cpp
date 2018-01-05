#include <ncurses.h>
#include <panel.h>
#include <sstream>

#include "nc_play.h"

const int invPerPage = 9;

void doInventory(Game &game) {
    wclear(subWindow);
    show_panel(subPanel);

    if (game.inventory.empty()) {
        mvwprintw(subWindow, 1, 3, "You are carrying:");
        mvwprintw(subWindow, 3, 3, "Nothing");
        mvwprintw(subWindow, 16, COLS-31, "Press any key to return");
        box(subWindow, 0, 0);
        update_panels();
        doupdate();
        wgetch(subWindow);
        hide_panel(subPanel);
        update_panels();
        doupdate();
        return;
    }


    unsigned curItem = 0;
    while (true) {
        wclear(subWindow);
        box(subWindow, 0, 0);
        mvwprintw(subWindow, 1, 3, "You are carrying:");
        mvwprintw(subWindow, 16, 3, "SPACE to use     E to Equip     Z to cancel");

        unsigned curPage = curItem / invPerPage;
        for (unsigned i = 0; i < invPerPage && i + curPage * invPerPage < game.inventory.size(); ++i) {
            auto item = game.inventory[i + curPage * invPerPage];

            std::stringstream ss;
            ss << (char)('1' + i) << ") ";

            if (item.qty == 1) {
                ss << toUpperFirst(game.getString(game.getProperty(item.itemIdent, itmArticle)));
                ss << game.getString(game.getProperty(item.itemIdent, itmSingular));
            } else {
                ss << item.qty << ' ';
                ss << game.getString(game.getProperty(item.itemIdent, itmPlural));
            }

            int paddingLength = 30 - ss.tellp();
            for (int j = 0; j < paddingLength; ++j) {
                ss << ' ';
            }

            if (game.getProperty(item.itemIdent, itmOnUse)) {
                ss << "   Usable";
            } else {
                ss << "         ";
            }

            if (game.getProperty(item.itemIdent, itmSlot)) {
                ss << "   " << toTitleCase(game.getString(game.getProperty(item.itemIdent, itmSlot)));
            }

            paddingLength = COLS-12 - ss.tellp();
            for (int j = 0; j < paddingLength; ++j) {
                ss << ' ';
            }
            if (curItem == i + curPage * invPerPage) wattrset(subWindow, A_REVERSE);
            mvwprintw(subWindow, i + 3, 3, ss.str().c_str());
            wattrset(subWindow, A_NORMAL);

        }

        std::uint32_t descString = game.getProperty(game.inventory[curItem].itemIdent, itmDescription);
        if (descString) {
            auto i = wrapString(game.getString(descString), COLS-12);
            mvwprintw(subWindow, 13, 3, i[0].c_str());
            if (i.size() > 1) {
                mvwprintw(subWindow, 14, 3, i[1].c_str());
            }
        }

        update_panels();
        doupdate();
        int key = toupper(wgetch(subWindow));

        if (key == 'Z') {
            break;
        } else if (key == KEY_ENTER || key == 'U' || key == ' ' || key == '\n' || key == '\r') {
            if (!game.getProperty(game.inventory[curItem].itemIdent, itmOnUse)) {
                continue;
            }
            game.useItem(curItem);
            addToOutput(game.getOutput());
            break;
        } else if (key == 'E') {
            if (!game.getProperty(game.inventory[curItem].itemIdent, itmSlot)) {
                continue;
            }
            // game.useItem(curItem);
            // addToOutput(game.getOutput());
            // break;
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

    hide_panel(subPanel);
    update_panels();
    doupdate();
}
