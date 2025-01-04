files = runtime/mod.c runtime/physics.c main.c utils_impl.c runtime/serializer.c 
flags = -std=gnu2x  -I /opt/homebrew/include -L /opt/homebrew/lib -pg 
debug = -g3 -fsanitize=address ./rusty/target/debug/librusty.a
release = -O2 ./rusty/target/release/librusty.a 
libs=  -l raylib -l profiler
make: $(files)
	cd rusty && cargo build 
	cd ..
	gcc $(files) $(flags) $(libs) $(debug)
release:$(files)
	cd rusty && cargo build --release
	cd ..
	gcc $(files) $(flags) $(libs) $(release)
profile: $(files)
	make 
	./a.out
	pprof --text a.out dump.txt > output.txt

profile_release: $(files)
	make release
	./a.out
	pprof --text a.out dump.txt > output.txt