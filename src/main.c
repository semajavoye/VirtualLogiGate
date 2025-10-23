#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Includes for SDL3
#define SDL_MAIN_USE_CALLBACKS 1 // use the callbacks instead of main()
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

// Include logic gate simulation
#include "logic.h"

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static TTF_Font *font = NULL;

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 600

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("VirtualLogiGate", "1.0", "com.example.virtuallogigate");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("VirtualLogiGate", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!TTF_Init()) {
        SDL_Log("Couldn't initialize SDL_ttf: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    #ifdef FONT_PATH
    font = TTF_OpenFont(FONT_PATH, 24);
    #endif

    if (!font) {
        SDL_Log("Couldn't load font: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetRenderLogicalPresentation(renderer, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    const int charsize = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;

    /* as you can see from this, rendering draws over whatever was drawn before it. */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);  /* black, full alpha */
    SDL_RenderClear(renderer);  /* start with a blank canvas. */

    // Create the title
    char title_buffer[27];
    size_t title_length = sizeof(title_buffer) - 1;
    SDL_snprintf(title_buffer, sizeof(title_buffer), "Virtual LogiGate Simulator",
                (unsigned long long)(SDL_GetTicks() / 1000));

    SDL_Color text_color = { 255, 255, 255, 255 }; /* white full alpha */
    SDL_Surface *text_surface = TTF_RenderText_Blended(font, title_buffer, title_length, text_color); /* anti-aliased text */

    if (text_surface) {
        SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);

        if (text_texture) {
            SDL_FRect dest_rect;
            dest_rect.w = (float) text_surface->w;
            dest_rect.h = (float) text_surface->h;
            dest_rect.x = (WINDOW_WIDTH - text_surface->w) / 2.0f;
            dest_rect.y = 400;

            SDL_RenderTexture(renderer, text_texture, NULL, &dest_rect);

            SDL_DestroyTexture(text_texture);
        }

        SDL_DestroySurface(text_surface);
    }

    SDL_RenderDebugTextFormat(renderer, (float) ((WINDOW_WIDTH - (charsize * 46)) / 2), 400, "(This program has been running for %" SDL_PRIu64 " seconds.)", SDL_GetTicks() / 1000);

    SDL_RenderPresent(renderer);  /* put it all on the screen! */

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    if (font) {
        TTF_CloseFont(font);
    }
    TTF_Quit();
}
