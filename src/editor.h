#ifndef EDITOR_H
#define EDITOR_H

#include "logic.h"
#include "camera.h"
#include <stddef.h>
#include <stdbool.h>

#include <SDL3/SDL.h>

// Gate pin model
typedef enum
{
    PIN_INPUT1 = 0,
    PIN_INPUT2 = 1,
    PIN_OUTPUT = 2
} GatePinType;

typedef struct
{
    float x, y;
    float width, height;
    struct Gate *gate; // logic
} EditorGate;

static EditorGate *gates = NULL;
static size_t gate_count = 0;
static size_t gate_capacity = 0;

static bool switch_placement_active = false;

struct VisualGate
{
    float x;
    float y;
    float width;
    float height;

    bool is_selected;
    bool is_dragging;

    struct Gate *gate; // Pointer to the underlying logic gate
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

    struct Wire *wire; // Pointer to the underlying logic wire
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
Camera *editor_get_camera(void);

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
// Select any editor object (wire, lamp, gate) at the given world coordinate.
// Returns 1 if an object was selected, 0 otherwise.
int editor_select_at(float world_x, float world_y, const Camera *camera);

// Create a lamp visual+logic at a world position
void editor_create_lamp(float world_x, float world_y);

// Lamp placement helpers (used by UI tools)
void editor_begin_lamp_placement(void);
void editor_cancel_lamp_placement(void);
int editor_is_lamp_placement_active(void);

// Gate / switch placement
void editor_begin_switch_placement(void);
void editor_cancel_switch_placement(void);
int editor_is_switch_placement_active(void);
void editor_create_switch(float world_x, float world_y);

// Force a signal propagation pass (updates gates, wires, lamps)
void editor_propagate_signals(void);

// Toggle selected switch (if selection is a switch)
void editor_toggle_selected_switch(void);

// Delete the currently selected object (wire/lamp/gate)
void editor_delete_selected(void);

#endif // EDITOR_H