#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_FOLDER        "build/"
#define SRC_FOLDER          "sources/"

#ifdef WINDOWS_DEBUG
    typedef struct Find_Result {
        int windows_sdk_version;
        wchar_t *windows_sdk_root;
        wchar_t *windows_sdk_um_library_path;
        wchar_t *windows_sdk_ucrt_library_path;
        
        wchar_t *vs_exe_path;
        wchar_t *vs_library_path;
    } Find_Result;

    extern Find_Result find_visual_studio_and_windows_sdk(void);
    extern char* wchar_to_char(wchar_t* wide_string);
    extern wchar_t* get_include_paths (Find_Result result);
    extern wchar_t* get_lib_paths (Find_Result result);
    extern wchar_t* get_cl_exe_path (Find_Result result);
#endif

int
main (int argc, char **argv)
{
#if !defined(WINDOWS_DEBUG)
// Unfortunatle the NOB_GO_REBUILD_URSELF Technology™ does not work with the overcompilated debug build command
    NOB_GO_REBUILD_URSELF(argc, argv);
#endif
    if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) return 1;
    Nob_Cmd cmd = { 0 };

#if defined(WASM) && defined(_WIN32)
    nob_cmd_append(
        &cmd, "emcc.bat", 
        SRC_FOLDER"main.c", "-o", BUILD_FOLDER"main.html",
        "-DGRAPHICS_API_OPENGL_ES2",
        "-s", "FULL_ES2=1",         
        "-s", "WASM=1",
        "-s", "ASYNCIFY", 
        "--shell-file", "./template.html"
    );
#elif defined(WINDOWS_DEBUG)
    Find_Result result = find_visual_studio_and_windows_sdk();
    char* include_paths = wchar_to_char(get_include_paths(result));
    char* lib_paths = wchar_to_char(get_lib_paths(result));

    _putenv_s("INCLUDE", include_paths);
    _putenv_s("LIB", lib_paths);

    char* cl_exec_path = wchar_to_char(get_cl_exe_path(result));
    nob_cmd_append(
        &cmd, cl_exec_path, 
        SRC_FOLDER"main.c",
        "/Fe:"BUILD_FOLDER"main.exe", 
        "/Zi", "/link", "/DEBUG",
        "gdi32.lib",
        "opengl32.lib"
    );
#elif defined(__MINGW32__)
    nob_cmd_append(
        &cmd, "gcc", 
        "-o", BUILD_FOLDER"main.exe", 
        SRC_FOLDER"main.c", 
        "-lgdi32",
        "-lopengl32"
    );
#endif

    if (!nob_cmd_run_sync(cmd)) return 1;
    cmd.count = 0;

#if defined(WASM) && defined(_WIN32)
    if (argc > 1 && (strcmp("run", argv[1]) == 0)) {
        nob_cmd_append(
            &cmd, "emrun.bat",
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