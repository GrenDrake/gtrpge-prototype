#include <iostream>

extern "C" {
#include "glk.h"
}
#include "play.h"

void gameloop();

void glk_main() {
    try {
        gameloop();
    } catch (PlayError &e) {
        std::cerr << "FATAL: " << e.what() << "\n";
    }

    return;
}
