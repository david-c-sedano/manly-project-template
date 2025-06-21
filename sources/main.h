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

#define RLGL_IMPLEMENTATION
#define RGFW_IMPLEMENTATION
#define RGFW_NO_GL_HEADER
#define STB_TRUETYPE_IMPLEMENTATION
#include "../single_headers/rlgl.h"
#include "../single_headers/RGFW.h"
#include "../single_headers/stb_truetype.h"

//////////////////////
// PROJECT SETTINGS //
/////////////////////
#define WINDOW_WIDTH            800
#define WINDOW_HEIGHT           600
#define WINDOW_CONFIG           RGFW_windowCenter | RGFW_windowNoResize
#define MAX_FPS                 60
#define BACKGROUND_COLOR        50,50,50,255
#define SELECTED_DEFAULT_FONT   "./resources/JetBrainsMono-SemiBold.ttf"

//////////////////////
// TYPE DEFINITIONS //
//////////////////////
typedef struct RGBA { unsigned char r, g, b, a; } RGBA;
typedef struct Quad { int left, right, top, bottom; } Quad;
typedef struct FontRenderData
{
    unsigned int gl_id;
    int atlas_width, atlas_height;
    unsigned char* atlas_data;
    Quad bboxes[128];
} FontRenderData;

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

/* ------------------------------------------
 implementations in `text_renderer.c` 
 ------------------------------------------ */
FontRenderData Load_TTF             (const char* path, float scale);
void DrawWords                      (FontRenderData font, const char* text, float x, float y, RGBA color);
int TextWidth                       (FontRenderData font, const char* text);
int TextHeight                      (FontRenderData font);

///////////////////
// USEFUL MACROS //
///////////////////
#define SET_TEXTURE_ANTIALIASING(GL_ID)\
    rlTextureParameters(GL_ID, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_LINEAR);\
    rlTextureParameters(GL_ID, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_LINEAR)

#define SET_TEXTURE_PIXELPERFECT(GL_ID)\
    rlTextureParameters(GL_ID, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_NEAREST);\
    rlTextureParameters(GL_ID, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_NEAREST)

#ifdef YOUR_PROJECT_NAME_IMPLEMENTATION

// make separate ".c" files and just include them here
#include "platform_abstraction.c"
#include "text_renderer.c"

#endif // YOUR_PROJECT_NAME_IMPLEMENTATION
#endif // YOUR_PROJECT_NAME