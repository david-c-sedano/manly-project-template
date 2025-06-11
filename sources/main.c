#define YOUR_PROJECT_NAME_IMPLEMENTATION
#include "main.h"

int
main (void)
{
    MakeWindow("real window");
    while (EventLoop())
    {
        rlClearScreenBuffers();

        rlBegin(RL_TRIANGLES);
        rlColor3f(1.0f, 0.0f, 0.0f); rlVertex2f(-0.6f, -0.75f);
        rlColor3f(0.0f, 1.0f, 0.0f); rlVertex2f(0.6f, -0.75f);
        rlColor3f(0.0f, 0.0f, 1.0f); rlVertex2f(0.0f, 0.75f);
        rlEnd();

        EventLoop_Cleanup();
    }
    
    Destroy_Window();
}