#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "window.h"
#include "logic.h"
#include "editor.h"
#include "ui.h"
#include "input.h"
#include "render_utils.h"
#include "actions.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static TTF_Font *font = NULL;
static UI *ui = NULL;
static UI *ingame_ui = NULL;
static InputHandler *input_handler = NULL;

// Function to update button positions based on window size
static void update_ui_layout(int window_w, int window_h)
{
    // Clear existing buttons
    ui_destroy(ui);
    ui = ui_create(font);
    ui_destroy(ingame_ui);
    ingame_ui = ui_create(font);
    
    // Re-add main menu buttons - centered based on actual window size
    ui_add_button(ui, window_w / 2.0f - 150, window_h / 2.0f - 60, 300, 50, "Start Simulation", on_start_simulation_clicked);
    ui_add_button(ui, window_w / 2.0f - 150, window_h / 2.0f + 10, 300, 50, "Quit", on_quit_clicked);

    // Re-add in-game UI buttons
    ui_add_button(ingame_ui, window_w - 190.0f, 10, 180, 40, "Quit to Menu", on_back_to_menu_clicked);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("VirtualLogiGate", "1.0", "com.example.virtuallogigate");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("VirtualLogiGate", WINDOW_WIDTH, WINDOW_HEIGHT, 
                                     SDL_WINDOW_RESIZABLE, &window, &renderer)) {
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

    // UI initialisieren
    ui = ui_create(font);
    ingame_ui = ui_create(font);
    
    // Get actual window size for responsive button positioning
    int window_w, window_h;
    SDL_GetWindowSizeInPixels(window, &window_w, &window_h);
    
    // Initial button layout
    update_ui_layout(window_w, window_h);
    
    // Input-Handler initialisieren
    input_handler = input_create(ui);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    // Handle window resize events
    if (event->type == SDL_EVENT_WINDOW_RESIZED) {
        int window_w, window_h;
        SDL_GetWindowSizeInPixels(window, &window_w, &window_h);
        update_ui_layout(window_w, window_h);
    }

    // Pass events to the appropriate UI based on current state
    UI *active_ui = (current_ui_state == UI_STATE_MAIN_MENU) ? ui : ingame_ui;
    
    // Handle input for the active UI
    if (event->type == SDL_EVENT_MOUSE_MOTION) {
        ui_handle_mouse_motion(active_ui, event->motion.x, event->motion.y);
    } else if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        ui_handle_mouse_click(active_ui, event->button.x, event->button.y);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    // Render UI based on current UIState
    switch (current_ui_state) {
        case UI_STATE_MAIN_MENU:
            // Titel rendern
            {
                SDL_Color white = { 255, 255, 255, 255 };
                render_text_centered(renderer, font, "Virtual LogiGate Simulator", 50, white);
            }
            ui_render(ui, renderer);
            break;
            
        case UI_STATE_INGAME:
            // Render the circuit editor/simulation view
            ui_render(ingame_ui, renderer);
            // TODO: Render your circuit editor here
            break;
    }

    SDL_RenderPresent(renderer);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    if (input_handler) {
        input_destroy(input_handler);
    }
    if (ui) {
        ui_destroy(ui);
    }
    if (ingame_ui) {
        ui_destroy(ingame_ui);
    }
    
    if (font) {
        TTF_CloseFont(font);
    }
    TTF_Quit();
}
