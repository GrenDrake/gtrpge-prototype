# Gren's Text RPG Engine (GTRPGE)

GTRPGE is an engine for RPG games using a choice-based (or gamebook like) design. Games can include a variety of different stats and damage types, all of which can be customized for each individual game.

As GTRPGE is still incomplete, no actual releases are available. The assembler (build) should compile under most C++ compilers. The interpreter (play) is slightly less portable as it also requires ncurses.

# Assembling Game Files

The assembler takes one argument: the filename of a project file describing the project to build.

```
./build demo.prj
```

Project files are plain text files with a simplistic format; each line contains a single whitespace-separated command. The ```files``` command specifies the names of input files (relative to the current directory) while the ```output``` directive specifies the name of the file to be created. If the ```output``` directive is omitted, the assembler will output ```game.bin```.

```
files demo.src/base.src demo.src/forest.src
output game.bin
```

# Running a Game

The game interpreter may take a single argument: the name of the game datafile to run. If this is omitted, it will try to run ```game.bin``` in the current directory.

```
./play game.bin
```


# License

This project is licensed under the GPL-3.0 license.
