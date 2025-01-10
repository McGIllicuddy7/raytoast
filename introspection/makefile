files = lib/cereal.c lib/cintro.c lib/main.c lib/serialize_intro.c lib/cexpection.c
flags = -fsanitize=address -std=c2x -g3
libs = -lm -I ./include
make: $(files)
	clang $(files) $(flags) $(libs)