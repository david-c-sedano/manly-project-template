#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_FOLDER        "build/"
#define SRC_FOLDER          "sources/"

/* -------------------------------------------------------------------------------------------------------------
// uncomment this to compile to WASM to work on browsers
#define COMPILE_TO_WASM

// only set these if `nob` fails to drive emscripten.
// for example: you have to set the whole path to `emcc.bat` and `emrun.bat` since Windows is goofy
#define EMCC_PATH   "emcc"
#define EMRUN_PATH  "emrun"
 ------------------------------------------------------------------------------------------------------------- */

int
main (int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);
    if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) return 1;
    Nob_Cmd cmd = { 0 };

#if defined(COMPILE_TO_WASM)
    nob_cmd_append(
        &cmd, EMCC_PATH, 
        SRC_FOLDER"main.c", "-o", BUILD_FOLDER"main.html",
        "-DGRAPHICS_API_OPENGL_ES2",
        "-s", "FULL_ES2=1",         
        "-s", "WASM=1",
        "-s", "ASYNCIFY", 
        "--shell-file", "./template.html"
    );
#elif defined(__MINGW32__)
    nob_cmd_append(
        &cmd, "gcc", 
        "-o", BUILD_FOLDER"main.exe", 
        SRC_FOLDER"main.c", 
        "-lgdi32",
        "-lopengl32"
    );
#elif defined(_MSC_VER) // DEBUG BUILD FOR WINDOWS, THIS IS EASIEST TO GET TO WORK WITH REMEDYBG
    nob_cmd_append(
        &cmd, "cl", 
        SRC_FOLDER"main.c", 
        "/Fe:"BUILD_FOLDER"main.exe", 
        "/Zi", "/link", "/DEBUG",
        "gdi32.lib",
        "opengl32.lib"
    );
#endif

    if (!nob_cmd_run_sync(cmd)) return 1;
    cmd.count = 0;

#if defined(COMPILE_TO_WASM)
    if (argc > 1 && (strcmp("run", argv[1]) == 0)) {
        nob_cmd_append(
            &cmd, EMRUN_PATH,
            "--no_browser", "--port", "8080",
            BUILD_FOLDER"main.html" 
        );

        if (!nob_cmd_run_sync(cmd)) return 1;
        cmd.count = 0;
    }
#else
    if (argc > 1 && (strcmp("run", argv[1]) == 0)) {
        nob_cmd_append(&cmd, BUILD_FOLDER"main.exe");
        if (!nob_cmd_run_sync(cmd)) return 1;
        cmd.count = 0;
    }
#endif

    return 0;
}