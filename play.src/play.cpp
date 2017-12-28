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

static void doInventory(Game &game, GameIO &io) {
    io.setWindow(GameIO::Options);
    io.clear();
    io.say("Use what item? (Z to cancel)");

    io.setWindow(GameIO::Sidebar);


    while (true) {
        int lastItem = 0;
        io.clear();
        io.say("You are carrying:\n\n");
        if (game.inventory.empty()) {
            io.say("Nothing");
        } else {
            int i = 1;
            for (Game::CarriedItem &ci : game.inventory) {
                ++lastItem;
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

        int key = io.getKey();
        if (key == 'Z') {
            io.setWindow(GameIO::Main);
            return;
        } else if (key >= '1' && key < '1' + lastItem) {
            io.clear();
            io.setWindow(GameIO::Main);
            game.useItem(key - '1');
            return;
        }
    }
}

void gameloop() {
    GameIO io;
    Game game;

    game.loadDataFromFile("game.bin");
    game.startGame();
    io.say(game.getOutput());

    while (true) {
        drawOptions(game, io);

        int key = io.getKey();
        if (key >= '1' && key <= '9') {
            game.doOption(key - '1');
            io.say(game.getOutput());
            drawStatus(game, io);
        } else if (key == 'I' && game.actionAllowed()) {
            doInventory(game, io);
        } else if (key == 'Q') {
            io.style(GameIO::Emphasis);
            io.say("\nGoodbye!\n");
            return;
        }
    }
}
