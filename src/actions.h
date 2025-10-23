#ifndef ACTIONS_H
#define ACTIONS_H

#include <SDL3/SDL.h>
#include "ui.h"

// Global state
extern AppUIState current_ui_state;

void on_start_simulation_clicked(void);
void on_quit_clicked(void);
void on_back_to_menu_clicked(void);

#endif // ACTIONS_H