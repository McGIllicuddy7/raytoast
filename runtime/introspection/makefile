files = lib/cereal.c lib/cintro.c lib/main.c lib/serialize_intro.c
flags = -fsanitize=address -std=c2x -g3
includes = -I ./include -I /opt/homebrew/include/ -L /opt/homebrew/lib
libs = -lm -l elf
make: $(files)
	clang $(files) $(includes) $(flags) $(libs)
