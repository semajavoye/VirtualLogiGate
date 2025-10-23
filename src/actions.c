#include "actions.h"

// Global state variable
AppUIState current_ui_state = UI_STATE_MAIN_MENU;
UI *ingame_ui = NULL;

void on_start_simulation_clicked(void) {
    SDL_Log("Starting simulation...");
    current_ui_state = UI_STATE_INGAME;
}

void on_quit_clicked(void)
{
    SDL_Log("Exiting.");
    SDL_Event quit_event;
    quit_event.type = SDL_EVENT_QUIT;
    SDL_PushEvent(&quit_event);
}

void on_back_to_menu_clicked(void) {
    SDL_Log("Returning to main menu...");
    current_ui_state = UI_STATE_MAIN_MENU;
}