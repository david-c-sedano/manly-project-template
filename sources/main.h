#ifndef YOUR_PROJECT_NAME
#define YOUR_PROJECT_NAME

//////////////
// INCLUDES //
/////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define RGFW_IMPLEMENTATION
#define RGFW_NO_GL_HEADER
#define RLGL_IMPLEMENTATION
#include "../single_headers/rlgl.h"
#include "../single_headers/RGFW.h"

//////////////////////
// PROJECT SETTINGS //
/////////////////////
#define WINDOW_WIDTH            800
#define WINDOW_HEIGHT           600
#define WINDOW_CONFIG           RGFW_windowCenter | RGFW_windowNoResize
#define MAX_FPS                 60

//////////////////////
// TYPE DEFINITIONS //
//////////////////////

/////////////
// GLOBALS //
/////////////
RGFW_window*    WINDOW;
double          START_TIME          = 0;
unsigned int    FPS                 = 0;
unsigned int    FRAMES              = 0;

//////////////////////////
// FUNCTION DEFINITIONS //
//////////////////////////

/* ------------------------------------------
 implementations in `platform_abstraction.c` 
 ------------------------------------------ */
void MakeWindow                     (const char* window_title);
bool EventLoop                      (void);
void EventLoop_Cleanup              (void);
void Destroy_Window                 (void);

#ifdef YOUR_PROJECT_NAME_IMPLEMENTATION

// make separate ".c" files and just include them here
#include "platform_abstraction.c"

#endif // YOUR_PROJECT_NAME_IMPLEMENTATION
#endif // YOUR_PROJECT_NAME