#ifndef EDITOR_H
#define EDITOR_H

#include "logic.h"
#include <stddef.h>
#include <stdbool.h>

#include <SDL3/SDL.h>

struct VisualGate
{
    float x;
    float y;
    float width;
    float height;

    bool is_selected;
    bool is_dragging;

    struct Gate *gate;  // Pointer to the underlying logic gate
};

struct VisualWire
{
    struct WireConnection source;
    struct WireConnection target;

    float start_x;
    float start_y;
    float end_x;
    float end_y;
    float thickness; // Just for visual representation

    // Connections

    
    float *bend_points; // Array of bend points (x, y pairs)
    size_t bend_point_count;

    struct Wire *wire;  // Pointer to the underlying logic wire
};

struct VisualLamp
{
    float x;
    float y;
    float radius;

    struct Lamp *lamp;
};

// Editor rendering
void editor_render(SDL_Renderer *renderer);

// Wire placement functions
void wire_placement_handle_click(SDL_Renderer *renderer, float x, float y);
void wire_placement_render(SDL_Renderer *renderer);
void wire_placement_clear(void);

#endif // EDITOR_H