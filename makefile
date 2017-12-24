include Make.glkterm

CXXFLAGS=-Wall -g -std=c++11 -pedantic

BUILD_OBJS=build.src/build.o build.src/lexer.o build.src/parser.o build.src/makebin.o build.src/data.o
BUILD_TARGET=build

PLAY_LIBS=$(GLKLIB) $(LINKLIBS)
PLAY_OBJS=play.src/play.o play.src/game.o play.src/game_donode.o play.src/gameio_glk.o play.src/glkstart.o
PLAY_TARGET=play

all: build play game.bin

$(BUILD_TARGET): $(BUILD_OBJS)
	$(CXX) $(BUILD_OBJS) -o $(BUILD_TARGET)

$(PLAY_TARGET): $(PLAY_OBJS)
	$(CXX) $(PLAY_OBJS) $(PLAY_LIBS) -o $(PLAY_TARGET)

game.bin: demo.src/*
	./build demo.src/*

clean:
	$(RM) build.src/*.o play.src/*.o game.bin $(BUILD_TARGET) $(PLAY_TARGET)

.PHONY: all clean