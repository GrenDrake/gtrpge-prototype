#include <cctype>
#include <fstream>
#include <iostream>
#include <ncurses.h>
#include <string>
#include <sstream>
#include <vector>

#include "play.h"

const int maxOutputBuffer = 4096;
char gamefile[64] = "game.bin";

std::string outputBuffer;
std::ofstream *transcript = nullptr;

void addToOutput(const std::string &text) {
    if (transcript) {
        *transcript << text;
    }
    if (outputBuffer[outputBuffer.size()-1] != '\n') {
        outputBuffer += '\n';
    }
    outputBuffer += text;
    if (outputBuffer.size() > maxOutputBuffer) {
        outputBuffer.erase(0, outputBuffer.size() - maxOutputBuffer);
    }
}

const std::string& getOutput() {
    return outputBuffer;
}

void drawStatus(Game &game) {
    int maxX = getmaxx(stdscr);
    bkgdset(A_NORMAL | COLOR_PAIR(colorStatus));
    move(0, 0); clrtoeol();

    if (!game.gameStarted) return;

    if (game.locationName) {
        mvprintw(0, 3, game.getNameOf(game.locationName).c_str());
    }
    mvprintw(0, maxX-20, toUpperFirst(game.getTimeString()).c_str());
}

static void drawCombatTracker(Game &game) {
    unsigned maxNameLength = 0;
    for (const auto &whoIdent : game.combatants) {
        auto length = game.getNameOf(whoIdent).size();
        if (length > maxNameLength) {
            maxNameLength = length;
        }
    }
    unsigned skillsToShow = 0;
    for (int i = 0; i < sklCount; ++i) {
        const SkillDef *skillDef = game.getSkillDef(i);
        if (skillDef != nullptr && skillDef->testFlags(sklOnTracker)) {
            ++skillsToShow;
        }
    }

    const unsigned trackerWidth = maxNameLength + 8 + skillsToShow * 10;
    const unsigned trackerHeight = game.combatants.size() + 1;
    const std::uint32_t skillTable = game.readWord(headerSkillTable);

    int maxX = 0, maxY = 0;
    getmaxyx(stdscr, maxY, maxX);
    const unsigned top = 1;
    const unsigned left = maxX - trackerWidth;

    bkgdset(A_NORMAL | COLOR_PAIR(colorDialog));
    for (unsigned y = 0; y < trackerHeight; ++y) {
        for (unsigned x = 0; x < trackerWidth; ++x) {
            mvaddch(y+top, x+left, ' ');
        }
        mvaddch(y+top, left, ACS_VLINE);
    }
    mvaddch(top, left, ACS_ULCORNER);
    mvaddch(top+trackerHeight-1, left, ACS_LLCORNER);

    attron(A_BOLD);
    mvprintw(top, left+4, "NAME");
    unsigned shownSkills = 0;
    for (int sklCounter = 0; sklCounter < sklCount; ++sklCounter) {
        const SkillDef *skillDef = game.getSkillDef(sklCounter);
        if (skillDef != nullptr && skillDef->testFlags(sklOnTracker)) {
            std::uint32_t nameAddr = game.readWord(skillTable + sklCounter*sklSize + sklName);
            mvprintw(top, left+6+maxNameLength+shownSkills*10, "%s", toTitleCase(game.getNameOf(nameAddr).substr(0,9)).c_str());
            ++shownSkills;
        }
    }
    attroff(A_BOLD);

    int whoCounter = 0;
    for (const auto &whoIdent : game.combatants) {
        if (whoCounter == game.currentCombatant) {
            mvaddch(top+whoCounter+1, left+2, '*');
        }
        mvprintw(top+whoCounter+1, left+4, "%s", toUpperFirst(game.getNameOf(whoIdent)).c_str());

        unsigned shownSkills = 0;
        for (int sklCounter = 0; sklCounter < sklCount; ++sklCounter) {
            const SkillDef *skillDef = game.getSkillDef(sklCounter);
            if (skillDef == nullptr) continue;
            if (skillDef->testFlags(sklOnTracker)) {
                if (skillDef->testFlags(sklVariable)) {
                    mvprintw(top+whoCounter+1, left+6+maxNameLength+shownSkills*10, "%d/%d",
                            game.getSkillCur(whoIdent, sklCounter), game.getSkillMax(whoIdent, sklCounter));
                } else {
                    mvprintw(top+whoCounter+1, left+6+maxNameLength+shownSkills*10, "%d",
                            game.getSkillMax(whoIdent, sklCounter));
                }
                ++shownSkills;
            }
        }

        ++whoCounter;
    }
}

void gameloop() {
    Game game;

    game.loadDataFromFile(gamefile);
    addToOutput(game.getOutput());

    while (true) {
        bkgdset(A_NORMAL | COLOR_PAIR(colorMain));
        clear();
        drawStatus(game);
        drawOutput(game);
        drawOptions(game);
        if (game.isInCombat()) {
            drawCombatTracker(game);
        }
        refresh();

        int key = toupper(getch());
        if (key >= '1' && key <= '9') {
            game.doOption(key - '1');
            addToOutput(game.getOutput());
        } else if (key == 'L') {
            if (transcript) {
                addToOutput("\n[Transcript off.]");
                delete transcript;
                transcript = nullptr;
            } else {
                std::string filename = getString("Transcript file name:", 32, "transcript.txt");
                if (filename.find_first_of("/\\:") != std::string::npos) {
                    std::stringstream ss;
                    ss << "\"" << filename << "\" is not a valid filename.";
                    showMessageBox(ss.str());
                } else {
                    transcript = new std::ofstream(filename);
                    addToOutput("\n[Transcript on.]");
                }
            }
        } else if (key == ' ') {
            if (game.options.size() == 1) {
                game.doOption(0);
                addToOutput(game.getOutput());
            }
        } else if (key == 'I' && game.actionAllowed()) {
            doInventory(game);
        } else if (key == 'C' && game.actionAllowed()) {
            doCharacter(game);
        } else if (key == 'Q') {
            if (getYesNo("Are you sure you want to quit?", false)) {
                return;
            }
        }
    }
}


int main(int argc, char *argv[]) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, true);
    start_color();
    curs_set(0);
	init_pair(colorOptions, COLOR_WHITE, COLOR_BLUE);  // for options window
	init_pair(colorMain,    COLOR_WHITE, COLOR_BLACK); // for main window
	init_pair(colorDialog,  COLOR_WHITE, COLOR_RED);  // for dialog window
	init_pair(colorStatus,  COLOR_BLACK, COLOR_WHITE); // for status window


    try {
        gameloop();
    } catch (PlayError &e) {
        endwin();
        std::cerr << "Fatal error occured: ";
        std::cerr << e.what() << "\n";
        return 1;
    }
    if (transcript) {
        delete transcript;
    }
    endwin();
    return 0;
}