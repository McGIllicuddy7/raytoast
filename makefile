files = runtime/mod.c runtime/physics.c main.c utils_impl.c 
flags = -g3 -O2  -std=gnu2x ./rusty/target/debug/librusty.a -I /opt/homebrew/include -L /opt/homebrew/lib
libs=  -l raylib
make: $(files)
	cd rusty && cargo build 
	cd ..
	gcc $(files) $(flags) $(libs)