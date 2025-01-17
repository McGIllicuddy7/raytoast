files = runtime/mod.c runtime/physics.c main.c utils_impl.c runtime/comps.c runtime/resources.c runtime/drawing.c
flags = -std=c2x  -I /opt/homebrew/include -L /opt/homebrew/lib -pg -I include 
debug = -g3 
release = -O2 
libs=  -l raylib -lm -lprofiler
make: $(files)
	gcc $(files) $(flags) $(libs) $(debug)
release:$(files)
	gcc $(files) $(flags) $(libs) $(release)
profile: $(files)
	make 
	./a.out
	pprof --text a.out dump.txt > output.txt

profile_release: $(files)
	make release
	./a.out
	pprof --text a.out dump.txt > output.txt
