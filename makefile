GLKPATH=.
GLKLIB=-lglkterm -lncurses

CXXFLAGS=-Wall -g -std=c++11 -pedantic

BUILD_OBJS=build.src/build.o build.src/lexer.o build.src/parser.o build.src/makebin.o
BUILD_TARGET=build

PLAY_OBJS=play.src/main.o play.src/play.o play.src/game.o play.src/gameio.o play.src/glkstart.o
PLAY_TARGET=play

all: build play game.bin

build: $(BUILD_OBJS)
	$(CXX) $(BUILD_OBJS) -o $(BUILD_TARGET)

play: $(PLAY_OBJS)
	$(CXX) $(PLAY_OBJS) -L$(GLKPATH) $(GLKLIB) -o $(PLAY_TARGET)

game.bin: gamesrc
	./build

clean:
	$(RM) build.src/*.o play.src/*.o game.bin $(BUILD_TARGET) $(PLAY_TARGET)

.PHONY: all clean