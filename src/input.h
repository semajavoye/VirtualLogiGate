#ifndef INPUT_H
#define INPUT_H

#include <SDL3/SDL.h>
#include "ui.h"

typedef struct
{
    UI *ui;
    float mouse_x;
    float mouse_y;
} InputHandler;

InputHandler *input_create(UI *ui);
void input_destroy(InputHandler *input);
void input_handle_event(InputHandler *input, SDL_Event *event);

#endif