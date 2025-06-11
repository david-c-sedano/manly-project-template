#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "sources/"

int
main (int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);
    if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) 
    {
        printf("something went wrong. check nob.c source.\n");
        return 1;
    }
    Nob_Cmd cmd = { 0 };

#if defined(__MINGW32__)
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

    if (!nob_cmd_run_sync(cmd)) 
    {
        printf("something went extremely wrong. check nob.c source\n");
        return 1;
    }
    cmd.count = 0;

    if (argc > 1 && (strcmp("run", argv[1]) == 0)) {
        nob_cmd_append(&cmd, BUILD_FOLDER"main.exe");
        if (!nob_cmd_run_sync(cmd))
        {
            printf("something else went wrong. check nob.c source\n");
            return 1;
        }
        cmd.count = 0;
    }

    return 0;
}