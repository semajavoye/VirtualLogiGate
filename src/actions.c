#include "actions.h"
#include "editor.h"

// Global state variable
AppUIState current_ui_state = UI_STATE_MAIN_MENU;
UI *ingame_ui = NULL;

void on_start_simulation_clicked(void)
{
    SDL_Log("Starting simulation...");
    current_ui_state = UI_STATE_INGAME;
    ingame_entry_time = SDL_GetTicks();
}

void on_quit_clicked(void)
{
    SDL_Log("Exiting.");
    SDL_Event quit_event;
    quit_event.type = SDL_EVENT_QUIT;
    SDL_PushEvent(&quit_event);
}

void on_back_to_menu_clicked(void)
{
    SDL_Log("Returning to main menu...");
    current_ui_state = UI_STATE_MAIN_MENU;
}

bool can_accept_ingame_input()
{
    return SDL_GetTicks() - ingame_entry_time >= 1000;
}

void on_place_lamp_clicked(void)
{
    if (editor_is_lamp_placement_active())
    {
        editor_cancel_lamp_placement();
        SDL_Log("Lamp placement cancelled.");
    }
    else
    {
        editor_begin_lamp_placement();
        SDL_Log("Lamp placement enabled. Click in the workspace to place a lamp.");
    }
}

void on_place_switch_clicked(void)
{
    if (editor_is_switch_placement_active())
    {
        editor_cancel_switch_placement();
        SDL_Log("Switch placement cancelled.");
    }
    else
    {
        editor_begin_switch_placement();
        SDL_Log("Switch placement enabled. Click to place a switch.");
    }
}
