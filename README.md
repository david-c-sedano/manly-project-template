# GRAPHICS PROGRAMMING FOR SCRUBS
Aside from CSTDLIB, OpenGL, and OS-Level libraries, ***the entirety of the source code for your video game/multimedia app is contained in this project template***. Just do a pattern search in VIM (or VSCode if you are a loser) and never read documentation again.

Thanks to:
- Tsoding's [nob.h](https://github.com/tsoding/nob.h) for allowing us to use the C preprocessor and programming language as the build system
- [RGFW](https://github.com/ColleagueRiley/RGFW) for allowing the sidestepping of WINAPI and X11 nonsense
- [Raylib](https://github.com/raysan5/raylib) (greatest open source project) for undoing the sin that is DirectX, Vulkan, modern OpenGL, and Metal.

Id also highly reccomend using stb_image as well so you don't have to write your own decompression algorithm.

This is a nice middle ground if Raylib is too hand-holdy for you, but modern graphics APIs and platforms make you hate programming. If you ever need that *finer grained control* just dive into the source code and figure out how to hack things.

# Usage
- Compile `nob.c` with your chosen C compiler
    - example: `gcc nob.c -o build.exe` or `cl nob.c /Fe:build.exe`
    - If you are compiling to WASM just uncomment the line in `nob.c` and it doesn't matter which C compiler you use
        - make sure `emcc` command works, of course
- run the resuling `.exe` file
    - give it an extra `run` flag to run it automatically, like `./build.exe run`
    - to run the "web application" if you are using emscripten, use `emrun --no_browser --port 8080 ./build/main.html`
- final executable located in `./build` directory

# Limitations
Only works on Windows with MinGW or MSVC, as well as on browsers via WASM.