# GRAPHICS PROGRAMMING FOR SCRUBS
Aside from CSTDLIB, OpenGL, and OS-Level libraries, ***the entirety of the source code for your video game/multimedia app is contained in this project template***. Just do a pattern search in VIM (or VSCode if you are a loser) and never read documentation again.

Thanks to:
- Tsoding's [nob.h](https://github.com/tsoding/nob.h) for allowing us to use the C preprocessor and programming language as the build system
- [RGFW](https://github.com/ColleagueRiley/RGFW) for allowing the sidestepping of WINAPI and X11 nonsense
- [Raylib](https://github.com/raysan5/raylib) (greatest open source project) for undoing the sin that is DirectX, Vulkan, modern OpenGL, and Metal.

Id also highly reccomend using stb_image as well so you don't have to write your own decompression algorithm.

This is a nice middle ground if Raylib is too hand-holdy for you, but modern graphics APIs and platforms make you hate programming. If you ever need that *finer grained control* just dive into the source code and figure out how to hack things.

# Usage
- Compile `nob.c` with `GCC`
    - `gcc nob.c -o build.exe`
    - If you are compiling to WASM just add `-DWASM` to the compiler command
- run the resuling `.exe` file
    - give it an extra `run` flag to run it automatically, like `./build.exe run`
    - to run the "web application" if you are using emscripten, use `emrun --no_browser --port 8080 ./build/main.html`
- final executable located in `./build` directory

# USAGE ON WINDOWS WITH MSVC
This mainly just exists for me I suppose, as I like to use RemedyDBG to help me with the **real nasty** problems, and it's difficult using that debugger with GCC. Unfortunately, doing any kind of low-level programming on Windows is a massive pain. Luckily I found some code online that will ***fight tooth and nail with Microsoft's obfuscation*** to find the paths required to use MSVC.

Due to the nature of `microsoft_craziness.cpp` being C++, extra steps are required to bootstrap the build system. These extra steps are unreasonable, but not as unreasonable as using Visual Studio properly. Just copy and paste each command

```
gcc -c nob.c -o nob.obj -DWINDOWS_DEBUG

g++ -c microsoft_craziness.cpp -o microsoft_craziness.obj -Wno-write-strings

g++ -o build.exe nob.obj microsoft_craziness.obj -ladvapi32 -loleaut32 -lole32
```

or just use the provided `debug_build.ps1` script. 

### But is this all a good idea though??
Yes because I dont have to fight with Make, CMake, Meson, or whatever wacky build tools and scripting languages

# Limitations
Only works on Windows with MinGW or MSVC (if you are real angry), as well as on browsers via WASM.