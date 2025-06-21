#define YOUR_PROJECT_NAME_IMPLEMENTATION
#include "main.h"

int
main (void)
{
    MakeWindow("real window");
    FontRenderData font = Load_TTF(SELECTED_DEFAULT_FONT, 64.0);

    while (EventLoop())
    {
        rlClearScreenBuffers();

        DrawWords(font, "Hello World!", 100, 100, (RGBA){255, 255, 255, 255});

        EventLoop_Cleanup();
    }
    
    Destroy_Window();
}
