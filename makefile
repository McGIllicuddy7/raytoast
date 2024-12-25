files = runtime/mod.c runtime/physics.c main.c utils_impl.c
flags = -g3 -O2  -std=gnu2x
libs=  /opt/homebrew/lib/libraylib.a -l m -framework Foundation -framework CoreFoundation -framework CoreAudio -framework CoreVideo -framework IOKit -framework Cocoa -framework OpenGl
make: $(files)
	gcc $(files) $(flags) $(libs)