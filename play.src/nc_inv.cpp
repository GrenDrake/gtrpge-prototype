#include <ncurses.h>
#include <panel.h>
#include <sstream>

#include "nc_play.h"

void doInventory(Game &game) {
    wclear(subWindow);
    show_panel(subPanel);
    box(subWindow, 0, 0);

    mvwprintw(subWindow, 1, 3, "You are carrying:");
    mvwprintw(subWindow, 16, COLS-3-28-5, "Use what item? (Z to cancel)");



    int counter = 0;
    for (auto &item : game.inventory) {
        std::stringstream ss;
        if (game.getProperty(item.itemIdent, itmOnUse)) {
            ss << (char)('A' + counter) << ") ";
        } else {
            ss << " ) ";
        }
        if (item.qty == 1) {
            ss << toUpperFirst(game.getString(game.getProperty(item.itemIdent, itmArticle)));
            ss << game.getString(game.getProperty(item.itemIdent, itmSingular));
        } else {
            ss << item.qty << ' ';
            ss << game.getString(game.getProperty(item.itemIdent, itmPlural));
        }

        int x = 0, y = 0;
        if (counter < 11) {
            x = 3;
            y = counter + 3;
        } else {
            x = (COLS-6) / 2;
            y = counter - 8;
        }
        mvwprintw(subWindow, y, x, ss.str().c_str());
        ++counter;

        if (counter >= 22) break;
    }

    update_panels();
    doupdate();

    while (true) {
        int key = toupper(getch());

        if (key == 'Z') {
            break;
        } else if (key >= 'A' && key <= 'V' && key < 'A' + static_cast<int>(game.inventory.size())) {
            key -= 'A';
            if (!game.getProperty(game.inventory[key].itemIdent, itmOnUse)) {
                continue;
            }
            std::stringstream ss;
            addToOutput(ss.str().c_str());
            game.useItem(key);
            addToOutput(game.getOutput());
            break;
        }
    }
    hide_panel(subPanel);
    update_panels();
    doupdate();
}
