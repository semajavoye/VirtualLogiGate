#include "input.h"
#include <stdlib.h>

InputHandler *input_create(UI *ui)
{
    InputHandler *input = malloc(sizeof(InputHandler));
    if (!input)
        return NULL;

    input->ui = ui;
    input->mouse_x = 0;
    input->mouse_y = 0;

    return input;
}

void input_destroy(InputHandler *input)
{
    if (input)
    {
        free(input);
    }
}

void input_handle_event(InputHandler *input, SDL_Event *event)
{
    if (event->type == SDL_EVENT_MOUSE_MOTION)
    {
        input->mouse_x = event->motion.x;
        input->mouse_y = event->motion.y;
        ui_handle_mouse_motion(input->ui, input->mouse_x, input->mouse_y);
    }
    else if (event->type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        if (event->button.button == SDL_BUTTON_LEFT)
        {
            ui_handle_mouse_click(input->ui, event->button.x, event->button.y);
        }
    }
}