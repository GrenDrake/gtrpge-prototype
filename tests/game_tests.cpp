#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../play.src/play.h"


TEST_CASE("Reading data from game memory", "[Game::read]") {
    uint8_t binData[] = {
        0x01, 0x02, 0x03, 0x04
    };
    Game game;
    game.setDataAs(binData, 4);

    REQUIRE(game.readByte(2) == 0x03);
    REQUIRE(game.readShort(1) == 0x0302);
    REQUIRE(game.readWord(0) == 0x04030201);
}
