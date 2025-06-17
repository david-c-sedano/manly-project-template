gcc -c nob.c -o nob.obj -DWINDOWS_DEBUG
g++ -c microsoft_craziness.cpp -o microsoft_craziness.obj -Wno-write-strings
g++ -o build.exe nob.obj microsoft_craziness.obj -ladvapi32 -loleaut32 -lole32
./build.exe