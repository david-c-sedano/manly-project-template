/* --------------------
    GLOBALS:
        WINDOW
        FPS
        FRAMES
        START_TIME
--------------------- */

void
MakeWindow (const char* window_title)
{
    WINDOW = RGFW_createWindow(window_title, RGFW_RECT(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_CONFIG);
    START_TIME = RGFW_getTime();
    rlLoadExtensions(RGFW_getProcAddress);
    rlglInit(WINDOW_WIDTH, WINDOW_HEIGHT);
    rlClearColor(255, 255, 255, 255);
}

bool
EventLoop (void)
{
    return (RGFW_window_shouldClose(WINDOW) == RGFW_FALSE);
}

void
EventLoop_Cleanup (void)
{
    rlDrawRenderBatchActive();
    RGFW_window_swapBuffers(WINDOW);
    RGFW_window_checkEvents(WINDOW, 0);
    FPS = RGFW_checkFPS(START_TIME, FRAMES, MAX_FPS);
    FRAMES++;
}

void
Destroy_Window (void)
{
    rlglClose();
    RGFW_window_close(WINDOW);
}