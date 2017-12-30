#include <string>

#include "play.h"

extern "C" {
#include "glk.h"
}

char gamefile[64] = "game.bin";

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
            io.say(game.getOutput());
            return;
        }
    }
}

static void doCharacter(Game &game, GameIO &io) {
    if (game.party.empty()) return;
    int curParty = 0;
    int curChar = game.party[curParty];

    io.setWindow(GameIO::Sidebar);
    io.clear();
    Character *c = game.getCharacter(curChar);
    io.say(game.getNameOf(curChar));
    io.say(" (");
    io.say(game.getNameOf(c->sex));
    io.say(" ");
    io.say(game.getNameOf(c->species));
    io.say(")\n");

    std::uint32_t skillTable = game.readWord(headerSkillTable);
    for (int i = 0; i < sklCount; ++i) {
        std::uint32_t nameAddr = game.readWord(skillTable + i*sklSize + sklName);
        if (!nameAddr) continue;

        const char *name = game.getString(nameAddr);
        io.say(i);
        io.say(") ");
        io.say(name);
        io.say(": ");
        if (game.testSkillFlags(i, sklVariable)) {
            io.say(game.getSkillCur(curChar, i));
            io.say("/");
        }
        io.say(game.getSkillMax(curChar, i));
        io.say("\n");
    }
    io.setWindow(GameIO::Main);
}

void gameloop() {
    GameIO io;
    Game game;

    try {
        game.loadDataFromFile(gamefile);
    } catch (PlayError &e) {
        io.say("FATAL ERROR: ");
        io.say(e.what());
        return;
    }

    game.startGame();
    io.say(game.getOutput());

    while (true) {
        drawOptions(game, io);

        int key = io.getKey();
        if (key >= '1' && key <= '9') {
            game.doOption(key - '1');
            io.say(game.getOutput());
            drawStatus(game, io);
        } else if (key == 'L') {
            if (io.hasTranscript()) {
                io.stopTranscript();
            } else {
                io.startTranscript();
            }
        } else if (key == 'I' && game.actionAllowed()) {
            doInventory(game, io);
        } else if (key == 'C') {
            doCharacter(game, io);
        } else if (key == 'Q') {
            io.style(GameIO::Emphasis);
            io.say("\nGoodbye!\n");
            return;
        }
    }
}
