files = runtime/mod.c main.c utils_impl.c
flags = -std=c2x -g3 
libs=  /usr/local/lib/libraylib.a -l m -std=c2x -framework Foundation -framework CoreFoundation -framework CoreAudio -framework CoreVideo -framework IOKit -framework Cocoa -framework OpenGl
make: $(files)
	gcc $(files) $(flags) $(libs)