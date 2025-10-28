#ifndef EDITOR_H
#define EDITOR_H

#include "logic.h"
#include "camera.h"
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

// Editor initialization and management
void editor_init(void);
Camera* editor_get_camera(void);

// Shutdown/cleanup editor resources
void editor_shutdown(void);

// Editor rendering
void editor_render(SDL_Renderer *renderer);

// Wire placement functions
// Start a new wire placement (called on first left-click)
void wire_placement_start(float world_x, float world_y);

// Add another bend point to the active wire (called on subsequent left-clicks)
void wire_placement_add_point(float world_x, float world_y);

// Finish the current wire placement (called on right-click)
void wire_placement_finish(void);

// Cancel the current wire placement
void wire_placement_cancel(void);

// Update the current pointer position for live preview (screen->world coords should be passed)
void wire_placement_update_pointer(float world_x, float world_y);

// Render placed wire points and the active preview (uses camera to transform to screen coords)
void wire_placement_render(SDL_Renderer *renderer, const Camera *camera);

// Clear all placed wires/points (reset)
void wire_placement_clear(void);

// Query whether a wire placement is currently active
int wire_placement_is_active(void);

// Multiple-wire management & selection
// Select a wire at the given world coordinate. Returns 1 if a wire was selected, 0 otherwise.
int editor_select_wire_at(float world_x, float world_y, const Camera *camera);

// Delete the currently selected wire (if any)
void editor_delete_selected(void);

#endif // EDITOR_H