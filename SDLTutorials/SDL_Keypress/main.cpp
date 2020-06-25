#include <SDL.h>
#include <stdio.h>
#include <string>

enum KeyPressSurfaces
{
    KEY_PRESS_SURFACE_DEFAULT,
    KEY_PRESS_SURFACE_UP,
    KEY_PRESS_SURFACE_DOWN,
    KEY_PRESS_SURFACE_LEFT,
    KEY_PRESS_SURFACE_RIGHT,
    KEY_PRESS_SURFACE_TOTAL
};

const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 240;

SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
bool init()
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        //Create window
        gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL)
        {
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            //Get window surface
            gScreenSurface = SDL_GetWindowSurface(gWindow);
        }
    }

    return success;
}

void close()
{
    //Destroy window
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    //Quit SDL subsystems
    SDL_Quit();
}

int main(int argc, char* args[])
{
    //Main loop flag
    bool quit = false;

    //Event handler
    SDL_Event e;

    if (!init())
    {
        printf("Failed to initialize!\n");
        return -1;
    }

    //Set default current surface
    //While application is running
    while (!quit)
    {
        //Handle events on queue
        while (SDL_PollEvent(&e) != 0)
        {
            //User requests quit
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
            //User presses a key
            else if (e.type == SDL_KEYDOWN)
            {
                //Select surfaces based on key press
                switch (e.key.keysym.sym)
                {
                case SDLK_UP:
                    printf("Up pressed\n");
                    break;

                case SDLK_DOWN:
                    printf("Down pressed\n");
                    break;

                case SDLK_LEFT:
                    printf("Left pressed\n");
                    break;

                case SDLK_RIGHT:
                    printf("Right pressed\n");
                    break;

                default:
                    break;
                }
            }
        }
    }
    close();
    return 0;
}
