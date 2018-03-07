CXXFLAGS=-Wall -g -std=c++11 -pedantic

BUILD_OBJS=build.src/build.o build.src/lexer.o build.src/parser.o \
		   build.src/makebin.o build.src/data.o build.src/project.o \
		   build.src/opcodes.o build.src/symboltable.o
BUILD_TARGET=./build

NCURSES_LIBS=-lncurses
NCURSES=play.src/curses/core.o play.src/curses/inventory.o \
		play.src/curses/charsheet.o play.src/curses/utility.o \
		play.src/curses/output.o

PLAY_LIBS=$(NCURSES_LIBS)
PLAY_UI=$(NCURSES)

PLAY_OBJS=$(PLAY_UI) play.src/textutils.o play.src/game.o \
			play.src/game_donode.o
PLAY_TARGET=./play

all: $(BUILD_TARGET) $(PLAY_TARGET) game.bin



$(BUILD_TARGET): $(BUILD_OBJS)
	$(CXX) $(BUILD_OBJS) -o $(BUILD_TARGET)

$(PLAY_TARGET): $(PLAY_OBJS)
	$(CXX) $(PLAY_OBJS) $(PLAY_LIBS) -o $(PLAY_TARGET)

game.bin: $(BUILD_TARGET) demo.prj demo.src/*
	$(BUILD_TARGET) demo.prj



tests: tests/text_tests tests/game_tests

tests/text_tests: tests/text_tests.o play.src/textutils.o
	$(CXX) tests/text_tests.o play.src/textutils.o -o tests/text_tests
	tests/text_tests

tests/game_tests: tests/game_tests.o play.src/game.o play.src/game_donode.o
	$(CXX) tests/game_tests.o play.src/game.o play.src/game_donode.o -o tests/game_tests
	tests/game_tests



clean:
	$(RM) build.src/*.o play.src/*.o play.src/curses/*.o tests/*.o game.bin $(BUILD_TARGET) $(PLAY_TARGET)

.PHONY: all clean tests