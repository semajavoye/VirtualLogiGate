#ifndef ACTIONS_H
#define ACTIONS_H

#include <SDL3/SDL.h>
#include "ui.h"

// Global state
extern AppUIState current_ui_state;

// Var to store the ingame entry in ms
static Uint64 ingame_entry_time = 0;

void on_start_simulation_clicked(void);
void on_quit_clicked(void);
void on_back_to_menu_clicked(void);

bool can_accept_ingame_input();

#endif // ACTIONS_H