#include <string>

#include "play.h"

extern "C" {
#include "glk.h"
}

static void drawStatus(Game &game, GameIO &io) {
    io.setWindow(GameIO::Status);
    io.clear();
    if (game.locationName) {
        io.say(game.getString(game.locationName));
    }
    io.setWindow(GameIO::Main);
}

static void drawOptions(Game &game, GameIO &io) {
    io.setWindow(GameIO::Options);
    io.clear();
    int num = 1;
    for (auto &option : game.options) {
        io.say(num++);
        io.say(") ");
        if (option.name == 1) {
            io.say("Continue");
        } else {
            io.say(game.getString(option.name));
        }
        io.say("\n");
    }

    io.setWindow(GameIO::Main);
}

static void drawInventory(Game &game, GameIO &io) {
    io.setWindow(GameIO::Sidebar);
    io.clear();
    io.say("You are carrying:\n\n");
    if (game.inventory.empty()) {
        io.say("Nothing");
    } else {
        int i = 1;
        for (CarriedItem &ci : game.inventory) {
            io.say(i++);
            io.say(") ");
            if (ci.qty == 1) {
                io.say(game.getString(game.getProperty(ci.itemIdent, itmArticle)));
                io.say(game.getString(game.getProperty(ci.itemIdent, itmSingular)));
            } else {
                io.say(ci.qty);
                io.say(" ");
                io.say(game.getString(game.getProperty(ci.itemIdent, itmPlural)));
            }
            io.say("\n");
        }
    }
    io.setWindow(GameIO::Main);
}

void gameloop() {
    enum SidebarMode {
        Blank, Inventory
    };
    GameIO io;
    Game game(io);
    enum SidebarMode mode = Blank;

    game.loadDataFromFile("game.bin");
    game.startGame();

    while (true) {
        drawOptions(game, io);
        switch(mode) {
            case Blank:
                break;
            case Inventory:
                drawInventory(game, io);
                break;
        }

        int key = io.getKey();
        if (key >= '1' && key <= '9') {
            game.doOption(key - '1');
            drawStatus(game, io);
        } else if (key == 'I' && game.actionAllowed()) {
            drawInventory(game, io);
            mode = Inventory;
        } else if (key == 'U' && game.actionAllowed()) {
            if (!game.inventory.empty()) {
                game.useItem(0);
            }
        } else if (key == 'Q') {
            io.style(GameIO::Emphasis);
            io.say("\nGoodbye!\n");
            return;
        }
    }
}
