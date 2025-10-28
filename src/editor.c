#include "editor.h"
#include "camera.h"
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <stdbool.h>

// Forward declarations for static functions
static void ensure_gates_capacity(void);
static int find_nearest_gate_pin(float world_x, float world_y, float max_distance, int *out_gate_index, GatePinType *out_pin);
static int hit_test_gate(float world_x, float world_y);
static void detach_gates_from_wire(struct Wire *logic_wire);
static void gate_pin_world(const EditorGate *eg, GatePinType pin, float *out_x, float *out_y);

// Global camera instance for the editor
static Camera editor_camera;

// Storage for wire points (in world coordinates)
typedef struct
{
    float x;
    float y;
} WirePoint;

static WirePoint *wire_points = NULL;
static size_t wire_point_count = 0;
static size_t wire_point_capacity = 0;

// Editor-level wire that also can hold a pointer to the logic-level struct Wire
typedef struct
{
    WirePoint *points; // dynamic array of points for this wire
    size_t count;
    size_t capacity;
    struct Wire *logic_wire; // pointer to logic model Wire (allocated when finished)
    // connection info: -1 = none, otherwise gate index and pin
    int start_gate_index;
    GatePinType start_pin;
    int end_gate_index;
    GatePinType end_pin;
} EditorWire;

static EditorWire *wires = NULL;
static size_t wire_count = 0;
static size_t wire_capacity = 0;

// Lamps
typedef struct
{
    float x, y;
    float radius;
    struct Lamp *logic_lamp;
} EditorLamp;

static EditorLamp *lamps = NULL;
static size_t lamp_count = 0;
static size_t lamp_capacity = 0;

static bool lamp_placement_active = false;

static const float LAMP_DEFAULT_RADIUS = 6.0f;
static const float LAMP_CONNECTION_RADIUS = 10.0f;

// Selection
typedef enum
{
    SELECT_NONE = 0,
    SELECT_WIRE = 1,
    SELECT_LAMP = 2
} SelectionType;
static SelectionType selected_type = SELECT_NONE;
static int selected_index = -1;
// Extend enum to include gates
// (keep backward compatibility by reusing same values)

// Wire placement state
static bool wire_active = false;     // Whether a wire is currently being placed
static float pointer_world_x = 0.0f; // Current pointer position in world coords (for preview)
static float pointer_world_y = 0.0f;

int rectangle_w = 10; // Width of grid rectangles in pixels (world space)
int rectangle_h = 10; // Height of grid rectangles in pixels (world space)

// int click_count = 0; // not needed with stateful implementation

void editor_init(void)
{
    camera_init(&editor_camera);
}

Camera *editor_get_camera(void)
{
    return &editor_camera;
}

// Internal helper to ensure capacity
static void ensure_wire_capacity(void)
{
    if (wire_point_count >= wire_point_capacity)
    {
        wire_point_capacity = wire_point_capacity == 0 ? 16 : wire_point_capacity * 2;
        wire_points = realloc(wire_points, wire_point_capacity * sizeof(WirePoint));
    }
}

static void ensure_wires_capacity(void)
{
    if (wire_count >= wire_capacity)
    {
        wire_capacity = wire_capacity == 0 ? 8 : wire_capacity * 2;
        wires = realloc(wires, wire_capacity * sizeof(EditorWire));
    }
}

static void ensure_lamps_capacity(void)
{
    if (lamp_count >= lamp_capacity)
    {
        lamp_capacity = lamp_capacity == 0 ? 8 : lamp_capacity * 2;
        lamps = realloc(lamps, lamp_capacity * sizeof(EditorLamp));
    }
}

static float distance_sq(float ax, float ay, float bx, float by)
{
    float dx = ax - bx;
    float dy = ay - by;
    return dx * dx + dy * dy;
}

static EditorLamp *find_lamp_near_point(float world_x, float world_y, float max_distance)
{
    float max_sq = max_distance * max_distance;
    for (size_t i = 0; i < lamp_count; ++i)
    {
        if (distance_sq(world_x, world_y, lamps[i].x, lamps[i].y) <= max_sq)
        {
            return &lamps[i];
        }
    }
    return NULL;
}

static EditorWire *find_wire_endpoint_near(float world_x, float world_y, float max_distance)
{
    float max_sq = max_distance * max_distance;
    for (size_t i = 0; i < wire_count; ++i)
    {
        EditorWire *w = &wires[i];
        if (w->count == 0)
            continue;
        if (distance_sq(world_x, world_y, w->points[0].x, w->points[0].y) <= max_sq)
        {
            return w;
        }
        if (w->count > 1 && distance_sq(world_x, world_y, w->points[w->count - 1].x, w->points[w->count - 1].y) <= max_sq)
        {
            return w;
        }
    }
    return NULL;
}

static void connect_lamp_to_wire(EditorLamp *lamp, EditorWire *wire)
{
    if (!lamp || !lamp->logic_lamp || !wire || !wire->logic_wire)
        return;
    lamp->logic_lamp->input = wire->logic_wire;
    lamp->logic_lamp->state = wire->logic_wire->state;
}

static void connect_wire_endpoints_to_lamps(EditorWire *wire)
{
    if (!wire || wire->count == 0)
        return;
    EditorLamp *start = find_lamp_near_point(wire->points[0].x, wire->points[0].y, LAMP_CONNECTION_RADIUS);
    if (start)
    {
        connect_lamp_to_wire(start, wire);
    }
    if (wire->count > 1)
    {
        EditorLamp *end = find_lamp_near_point(wire->points[wire->count - 1].x, wire->points[wire->count - 1].y, LAMP_CONNECTION_RADIUS);
        if (end && end != start)
        {
            connect_lamp_to_wire(end, wire);
        }
    }
}

static void connect_lamp_to_nearby_wire(EditorLamp *lamp)
{
    if (!lamp)
        return;
    EditorWire *wire = find_wire_endpoint_near(lamp->x, lamp->y, LAMP_CONNECTION_RADIUS);
    if (wire)
    {
        connect_lamp_to_wire(lamp, wire);
    }
    else if (lamp->logic_lamp)
    {
        lamp->logic_lamp->input = NULL;
        lamp->logic_lamp->state = UNKNOWN;
    }
}

static void detach_lamps_from_wire(struct Wire *logic_wire)
{
    if (!logic_wire)
        return;
    for (size_t i = 0; i < lamp_count; ++i)
    {
        if (!lamps[i].logic_lamp)
            continue;
        if (lamps[i].logic_lamp->input == logic_wire)
        {
            lamps[i].logic_lamp->input = NULL;
            lamps[i].logic_lamp->state = UNKNOWN;
        }
    }
}

// Snap world position to grid
static void snap_to_grid(float world_x, float world_y, int *out_x, int *out_y)
{
    *out_x = ((int)(world_x + rectangle_w / 2) / rectangle_w) * rectangle_w;
    *out_y = ((int)(world_y + rectangle_h / 2) / rectangle_h) * rectangle_h;
}

void editor_begin_lamp_placement(void)
{
    lamp_placement_active = true;
    wire_placement_cancel();
}

void editor_cancel_lamp_placement(void)
{
    lamp_placement_active = false;
}

int editor_is_lamp_placement_active(void)
{
    return lamp_placement_active ? 1 : 0;
}

void editor_begin_switch_placement(void)
{
    switch_placement_active = true;
    lamp_placement_active = false;
    wire_placement_cancel();
}

void editor_cancel_switch_placement(void)
{
    switch_placement_active = false;
}

int editor_is_switch_placement_active(void)
{
    return switch_placement_active ? 1 : 0;
}

void editor_create_switch(float world_x, float world_y)
{
    ensure_gates_capacity();
    int sx, sy;
    snap_to_grid(world_x, world_y, &sx, &sy);
    EditorGate *g = &gates[gate_count++];
    g->x = (float)sx;
    g->y = (float)sy;
    g->width = 20.0f;
    g->height = 14.0f;
    g->gate = malloc(sizeof(struct Gate));
    if (g->gate)
    {
        g->gate->type = CONSTANT_LOW; // default off
        g->gate->input1 = NULL;
        g->gate->input2 = NULL;
        g->gate->output = NULL;
    }
    // try to connect to nearby wires
    connect_lamp_to_nearby_wire((EditorLamp *)g); // reuse logic: this is safe since pointer types differ; but instead just try wires
    switch_placement_active = false;
}

// Start a new wire
void wire_placement_start(float world_x, float world_y)
{
    wire_placement_clear();
    wire_active = true;
    wire_placement_update_pointer(world_x, world_y);
    // add first point
    wire_placement_add_point(world_x, world_y);
}

// Add a new bend point
void wire_placement_add_point(float world_x, float world_y)
{
    if (!wire_active)
    {
        // If no active wire, start one instead
        wire_placement_start(world_x, world_y);
        return;
    }

    int sx, sy;
    snap_to_grid(world_x, world_y, &sx, &sy);
    ensure_wire_capacity();
    wire_points[wire_point_count].x = (float)sx;
    wire_points[wire_point_count].y = (float)sy;
    wire_point_count++;
}

// Finish the current wire (right click)
void wire_placement_finish(void)
{
    if (!wire_active)
        return;
    wire_active = false;
    // Convert current wire_points array into a Wire and store it
    if (wire_point_count > 0)
    {
        ensure_wires_capacity();
        EditorWire *w = &wires[wire_count];
        w->count = wire_point_count;
        w->capacity = wire_point_count;
        w->points = malloc(sizeof(WirePoint) * w->capacity);
        for (size_t i = 0; i < wire_point_count; ++i)
        {
            w->points[i] = wire_points[i];
        }
        // allocate logic-level wire and default state
        w->logic_wire = malloc(sizeof(struct Wire));
        if (w->logic_wire)
        {
            w->logic_wire->state = UNKNOWN;
        }
        connect_wire_endpoints_to_lamps(w);
        // Also try to connect to nearby gates (endpoints)
        // find gates near start/end and attach as inputs/outputs using simple heuristics
        if (w->count > 0)
        {
            float sx = w->points[0].x;
            float sy = w->points[0].y;
            float ex = w->points[w->count - 1].x;
            float ey = w->points[w->count - 1].y;
            // try pin-based connections
            int gate_idx;
            GatePinType pin;
            if (find_nearest_gate_pin(sx, sy, LAMP_CONNECTION_RADIUS, &gate_idx, &pin))
            {
                // attach this wire to that gate pin
                if (pin == PIN_INPUT1)
                    gates[gate_idx].gate->input1 = w->logic_wire;
                else if (pin == PIN_INPUT2)
                    gates[gate_idx].gate->input2 = w->logic_wire;
                else if (pin == PIN_OUTPUT)
                    gates[gate_idx].gate->output = w->logic_wire;
                w->start_gate_index = gate_idx;
                w->start_pin = pin;
            }
            else
            {
                w->start_gate_index = -1;
            }

            if (find_nearest_gate_pin(ex, ey, LAMP_CONNECTION_RADIUS, &gate_idx, &pin))
            {
                if (pin == PIN_INPUT1)
                    gates[gate_idx].gate->input1 = w->logic_wire;
                else if (pin == PIN_INPUT2)
                    gates[gate_idx].gate->input2 = w->logic_wire;
                else if (pin == PIN_OUTPUT)
                    gates[gate_idx].gate->output = w->logic_wire;
                w->end_gate_index = gate_idx;
                w->end_pin = pin;
            }
            else
            {
                w->end_gate_index = -1;
            }
        }
        wire_count++;
    }
    // clear temporary placement buffer but keep stored wires
    if (wire_points)
    {
        free(wire_points);
        wire_points = NULL;
    }
    wire_point_count = 0;
    wire_point_capacity = 0;
}

// Cancel current placement and discard points
void wire_placement_cancel(void)
{
    wire_active = false;
    wire_placement_clear();
}

// Update pointer for live preview (world coords)
void wire_placement_update_pointer(float world_x, float world_y)
{
    pointer_world_x = world_x;
    pointer_world_y = world_y;
}

int wire_placement_is_active(void)
{
    return wire_active ? 1 : 0;
}

void wire_placement_render(SDL_Renderer *renderer, const Camera *camera)
{
    // Render all stored wire points and preview segments
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Draw points (small squares) and connecting lines
    for (size_t i = 0; i < wire_point_count; i++)
    {
        float sx, sy;
        camera_world_to_screen(camera, wire_points[i].x, wire_points[i].y, &sx, &sy);
        // Draw small 3x3 square for visibility
        SDL_RenderFillRect(renderer, &(SDL_FRect){sx - 1.5f, sy - 1.5f, 3.0f, 3.0f});

        // Draw line to next point
        if (i + 1 < wire_point_count)
        {
            float nx, ny;
            camera_world_to_screen(camera, wire_points[i + 1].x, wire_points[i + 1].y, &nx, &ny);
            SDL_RenderLine(renderer, sx, sy, nx, ny);
        }
    }

    // If currently placing a wire and there is at least one point, draw preview segment
    if (wire_active && wire_point_count > 0)
    {
        float last_x = wire_points[wire_point_count - 1].x;
        float last_y = wire_points[wire_point_count - 1].y;
        int snap_x, snap_y;
        snap_to_grid(pointer_world_x, pointer_world_y, &snap_x, &snap_y);

        float sx, sy, px, py;
        camera_world_to_screen(camera, last_x, last_y, &sx, &sy);
        camera_world_to_screen(camera, (float)snap_x, (float)snap_y, &px, &py);

        // dashed line could be implemented later - for now a simple line
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderLine(renderer, sx, sy, px, py);
        // draw preview point
        SDL_RenderFillRect(renderer, &(SDL_FRect){px - 1.5f, py - 1.5f, 3.0f, 3.0f});
        // reset color
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    }
}

void wire_placement_clear(void)
{
    wire_point_count = 0;
    wire_active = false;
    if (wire_points)
    {
        free(wire_points);
        wire_points = NULL;
    }
    wire_point_capacity = 0;
}

// Helper: free all stored wires
static void free_all_wires(void)
{
    for (size_t i = 0; i < wire_count; ++i)
    {
        free(wires[i].points);
        if (wires[i].logic_wire)
        {
            detach_lamps_from_wire(wires[i].logic_wire);
            free(wires[i].logic_wire);
        }
    }
    free(wires);
    wires = NULL;
    wire_count = 0;
    wire_capacity = 0;
    selected_type = SELECT_NONE;
    selected_index = -1;
}

static void free_all_lamps(void)
{
    for (size_t i = 0; i < lamp_count; ++i)
    {
        if (lamps[i].logic_lamp)
            free(lamps[i].logic_lamp);
    }
    free(lamps);
    lamps = NULL;
    lamp_count = 0;
    lamp_capacity = 0;
}

void editor_shutdown(void)
{
    wire_placement_clear();
    free_all_wires();
    free_all_lamps();
    lamp_placement_active = false;
    // free gates
    for (size_t i = 0; i < gate_count; ++i)
    {
        if (gates[i].gate)
            free(gates[i].gate);
    }
    free(gates);
    gates = NULL;
    gate_count = 0;
    gate_capacity = 0;
}

// Compute squared distance from point to segment (ax,ay)-(bx,by)
static float point_segment_distance_sq(float px, float py, float ax, float ay, float bx, float by)
{
    float vx = bx - ax;
    float vy = by - ay;
    float wx = px - ax;
    float wy = py - ay;
    float c1 = vx * wx + vy * wy;
    if (c1 <= 0)
        return wx * wx + wy * wy;
    float c2 = vx * vx + vy * vy;
    if (c2 <= c1)
    {
        float dx = px - bx;
        float dy = py - by;
        return dx * dx + dy * dy;
    }
    float b = c1 / c2;
    float projx = ax + b * vx;
    float projy = ay + b * vy;
    float dx = px - projx;
    float dy = py - projy;
    return dx * dx + dy * dy;
}

// Hit-test wires: returns index of wire hit or -1
static int hit_test_wire(float world_x, float world_y)
{
    const float pick_radius = 8.0f; // world-space tolerance
    float pick_sq = pick_radius * pick_radius;
    for (size_t i = 0; i < wire_count; ++i)
    {
        EditorWire *w = &wires[i];
        if (w->count < 2)
            continue;
        for (size_t s = 0; s + 1 < w->count; ++s)
        {
            if (point_segment_distance_sq(world_x, world_y, w->points[s].x, w->points[s].y, w->points[s + 1].x, w->points[s + 1].y) <= pick_sq)
            {
                return (int)i;
            }
        }
    }
    return -1;
}

// Hit-test lamps: returns lamp index or -1
static int hit_test_lamp(float world_x, float world_y)
{
    for (size_t i = 0; i < lamp_count; ++i)
    {
        float dx = world_x - lamps[i].x;
        float dy = world_y - lamps[i].y;
        float r = lamps[i].radius;
        if (dx * dx + dy * dy <= r * r)
            return (int)i;
    }
    return -1;
}

int editor_select_at(float world_x, float world_y, const Camera *camera)
{
    (void)camera;
    // Try lamps first
    int li = hit_test_lamp(world_x, world_y);
    if (li >= 0)
    {
        selected_type = SELECT_LAMP;
        selected_index = li;
        return 1;
    }
    // gates
    int gi = hit_test_gate(world_x, world_y);
    if (gi >= 0)
    {
        selected_type = SELECT_WIRE + 1; // SELECT_LAMP=2; set 3 for gate
        selected_index = gi;
        return 1;
    }
    int wi = hit_test_wire(world_x, world_y);
    if (wi >= 0)
    {
        selected_type = SELECT_WIRE;
        selected_index = wi;
        return 1;
    }
    selected_type = SELECT_NONE;
    selected_index = -1;
    return 0;
}

void editor_delete_selected(void)
{
    if (selected_type == SELECT_WIRE)
    {
        if (selected_index < 0 || (size_t)selected_index >= wire_count)
            return;
        // free logic and points
        if (wires[selected_index].logic_wire)
        {
            detach_lamps_from_wire(wires[selected_index].logic_wire);
            free(wires[selected_index].logic_wire);
        }
        free(wires[selected_index].points);
        for (size_t i = selected_index; i + 1 < wire_count; ++i)
            wires[i] = wires[i + 1];
        wire_count--;
        selected_type = SELECT_NONE;
        selected_index = -1;
        return;
    }
    else if (selected_type == SELECT_LAMP)
    {
        if (selected_index < 0 || (size_t)selected_index >= lamp_count)
            return;
        if (lamps[selected_index].logic_lamp)
            free(lamps[selected_index].logic_lamp);
        // remove lamp
        for (size_t i = selected_index; i + 1 < lamp_count; ++i)
            lamps[i] = lamps[i + 1];
        lamp_count--;
        selected_type = SELECT_NONE;
        selected_index = -1;
        return;
    }
    else if (selected_type == SELECT_WIRE + 1)
    {
        // gate
        if (selected_index < 0 || (size_t)selected_index >= gate_count)
            return;
        if (gates[selected_index].gate)
        {
            // detach any wires that reference this gate
            for (size_t i = 0; i < wire_count; ++i)
            {
                EditorWire *w = &wires[i];
                if (w->logic_wire)
                {
                    detach_gates_from_wire(w->logic_wire);
                }
            }
            free(gates[selected_index].gate);
        }
        for (size_t i = selected_index; i + 1 < gate_count; ++i)
            gates[i] = gates[i + 1];
        gate_count--;
        selected_type = SELECT_NONE;
        selected_index = -1;
        return;
    }
}

void editor_toggle_selected_switch(void)
{
    if (selected_type != SELECT_WIRE + 1)
        return;
    if (selected_index < 0 || (size_t)selected_index >= gate_count)
        return;
    EditorGate *g = &gates[selected_index];
    if (!g->gate)
        return;
    // toggle between CONSTANT_LOW and CONSTANT_HIGH
    if (g->gate->type == CONSTANT_LOW)
        g->gate->type = CONSTANT_HIGH;
    else if (g->gate->type == CONSTANT_HIGH)
        g->gate->type = CONSTANT_LOW;
    // update output wire state
    if (g->gate->output)
    {
        g->gate->output->state = (g->gate->type == CONSTANT_HIGH) ? HIGH : LOW;
    }
    editor_propagate_signals();
}

void editor_propagate_signals(void)
{
    // Iteratively update gates until stable or max iterations reached
    const int MAX_ITER = 64;
    int iter = 0;
    int changed = 0;
    do
    {
        changed = 0;
        for (size_t i = 0; i < gate_count; ++i)
        {
            if (!gates[i].gate)
                continue;
            // capture previous outputs for comparison
            SignalState prev_out = gates[i].gate->output ? gates[i].gate->output->state : UNKNOWN;
            update_gate(gates[i].gate);
            SignalState new_out = gates[i].gate->output ? gates[i].gate->output->state : UNKNOWN;
            if (prev_out != new_out)
                changed = 1;
        }
        iter++;
    } while (changed && iter < MAX_ITER);

    // After convergence, sync lamp states
    for (size_t i = 0; i < lamp_count; ++i)
    {
        if (lamps[i].logic_lamp && lamps[i].logic_lamp->input)
        {
            lamps[i].logic_lamp->state = lamps[i].logic_lamp->input->state;
        }
        else if (lamps[i].logic_lamp)
        {
            lamps[i].logic_lamp->state = UNKNOWN;
        }
    }
}

// Main editor rendering function
void editor_render(SDL_Renderer *renderer)
{
    // Render the grid with camera transformation
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    int screen_w, screen_h;
    SDL_GetCurrentRenderOutputSize(renderer, &screen_w, &screen_h);

    // Calculate visible world bounds
    float world_left, world_top, world_right, world_bottom;
    camera_screen_to_world(&editor_camera, 0, 0, &world_left, &world_top);
    camera_screen_to_world(&editor_camera, screen_w, screen_h, &world_right, &world_bottom);

    // Calculate grid line start/end positions (snap to grid)
    int grid_start_x = ((int)world_left / rectangle_w) * rectangle_w;
    int grid_start_y = ((int)world_top / rectangle_h) * rectangle_h;
    int grid_end_x = ((int)world_right / rectangle_w + 1) * rectangle_w;
    int grid_end_y = ((int)world_bottom / rectangle_h + 1) * rectangle_h;

    // Render horizontal grid lines
    for (int world_y = grid_start_y; world_y <= grid_end_y; world_y += rectangle_h)
    {
        float screen_y_start, screen_y_end, screen_x_start, screen_x_end;
        camera_world_to_screen(&editor_camera, world_left, world_y, &screen_x_start, &screen_y_start);
        camera_world_to_screen(&editor_camera, world_right, world_y, &screen_x_end, &screen_y_end);
        SDL_RenderLine(renderer, screen_x_start, screen_y_start, screen_x_end, screen_y_end);
    }

    // Render vertical grid lines
    for (int world_x = grid_start_x; world_x <= grid_end_x; world_x += rectangle_w)
    {
        float screen_x_start, screen_x_end, screen_y_start, screen_y_end;
        camera_world_to_screen(&editor_camera, world_x, world_top, &screen_x_start, &screen_y_start);
        camera_world_to_screen(&editor_camera, world_x, world_bottom, &screen_x_end, &screen_y_end);
        SDL_RenderLine(renderer, screen_x_start, screen_y_start, screen_x_end, screen_y_end);
    }

    // Render all gates
    for (size_t i = 0; i < gate_count; ++i)
    {
        float gx = gates[i].x;
        float gy = gates[i].y;
        float gw = gates[i].width;
        float gh = gates[i].height;
        float sx, sy;
        camera_world_to_screen(&editor_camera, gx, gy, &sx, &sy);
        // approximate screen size for width/height scaling
        float sx2, sy2;
        camera_world_to_screen(&editor_camera, gx + gw, gy + gh, &sx2, &sy2);
        SDL_FRect rect = {sx, sy, sx2 - sx, sy2 - sy};
        SDL_SetRenderDrawColor(renderer, 100, 100, 160, 255);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
        SDL_RenderRect(renderer, &rect);
        // pin markers
        float px, py, px2, py2;
        gate_pin_world(&gates[i], PIN_INPUT1, &px, &py);
        camera_world_to_screen(&editor_camera, px, py, &px, &py);
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderFillRect(renderer, &(SDL_FRect){px - 2.5f, py - 2.5f, 5.0f, 5.0f});
        gate_pin_world(&gates[i], PIN_INPUT2, &px2, &py2);
        camera_world_to_screen(&editor_camera, px2, py2, &px2, &py2);
        SDL_RenderFillRect(renderer, &(SDL_FRect){px2 - 2.5f, py2 - 2.5f, 5.0f, 5.0f});
        gate_pin_world(&gates[i], PIN_OUTPUT, &px, &py);
        camera_world_to_screen(&editor_camera, px, py, &px, &py);
        SDL_RenderFillRect(renderer, &(SDL_FRect){px - 2.5f, py - 2.5f, 5.0f, 5.0f});
    }

    // Render all wires
    // Render stored wires
    for (size_t i = 0; i < wire_count; ++i)
    {
        EditorWire *w = &wires[i];
        if (w->count < 1)
            continue;
        // pick color: selected wires highlighted
        if (selected_type == SELECT_WIRE && (int)i == selected_index)
        {
            SDL_SetRenderDrawColor(renderer, 255, 130, 130, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
        }
        for (size_t s = 0; s < w->count; ++s)
        {
            float sx, sy;
            camera_world_to_screen(&editor_camera, w->points[s].x, w->points[s].y, &sx, &sy);
            // draw point marker
            SDL_RenderFillRect(renderer, &(SDL_FRect){sx - 1.5f, sy - 1.5f, 3.0f, 3.0f});
            if (s + 1 < w->count)
            {
                float nx, ny;
                camera_world_to_screen(&editor_camera, w->points[s + 1].x, w->points[s + 1].y, &nx, &ny);
                SDL_RenderLine(renderer, sx, sy, nx, ny);
            }
        }
        // draw endpoint connection indicators if connected to gate pins
        if (w->start_gate_index >= 0)
        {
            float px, py;
            gate_pin_world(&gates[w->start_gate_index], w->start_pin, &px, &py);
            float sxp, syp;
            camera_world_to_screen(&editor_camera, px, py, &sxp, &syp);
            SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
            SDL_RenderFillRect(renderer, &(SDL_FRect){sxp - 3.0f, syp - 3.0f, 6.0f, 6.0f});
        }
        if (w->end_gate_index >= 0)
        {
            float px, py;
            gate_pin_world(&gates[w->end_gate_index], w->end_pin, &px, &py);
            float sxp, syp;
            camera_world_to_screen(&editor_camera, px, py, &sxp, &syp);
            SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
            SDL_RenderFillRect(renderer, &(SDL_FRect){sxp - 3.0f, syp - 3.0f, 6.0f, 6.0f});
        }
    }

    // Render wire points being placed
    wire_placement_render(renderer, &editor_camera);

    // Render lamps
    for (size_t i = 0; i < lamp_count; ++i)
    {
        float sx, sy;
        camera_world_to_screen(&editor_camera, lamps[i].x, lamps[i].y, &sx, &sy);

        SignalState state = UNKNOWN;
        if (lamps[i].logic_lamp)
        {
            if (lamps[i].logic_lamp->input)
            {
                lamps[i].logic_lamp->state = lamps[i].logic_lamp->input->state;
            }
            else
            {
                lamps[i].logic_lamp->state = UNKNOWN;
            }
            state = lamps[i].logic_lamp->state;
        }

        SDL_Color color;
        switch (state)
        {
        case HIGH:
            color = (SDL_Color){255, 230, 60, 255};
            break;
        case LOW:
            color = (SDL_Color){90, 90, 90, 255};
            break;
        case UNKNOWN:
        default:
            color = (SDL_Color){140, 140, 180, 255};
            break;
        }

        bool is_selected = (selected_type == SELECT_LAMP && (int)i == selected_index);
        if (is_selected)
        {
            color.r = (Uint8)((color.r + 255) / 2);
            color.g = (Uint8)((color.g + 160) / 2);
            color.b = (Uint8)((color.b + 160) / 2);
        }

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &(SDL_FRect){sx - lamps[i].radius, sy - lamps[i].radius, lamps[i].radius * 2.0f, lamps[i].radius * 2.0f});
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderRect(renderer, &(SDL_FRect){sx - lamps[i].radius, sy - lamps[i].radius, lamps[i].radius * 2.0f, lamps[i].radius * 2.0f});
    }

    if (lamp_placement_active)
    {
        int snap_x, snap_y;
        snap_to_grid(pointer_world_x, pointer_world_y, &snap_x, &snap_y);
        float sx, sy;
        camera_world_to_screen(&editor_camera, (float)snap_x, (float)snap_y, &sx, &sy);
        SDL_SetRenderDrawColor(renderer, 255, 220, 120, 180);
        SDL_RenderRect(renderer, &(SDL_FRect){sx - LAMP_DEFAULT_RADIUS, sy - LAMP_DEFAULT_RADIUS, LAMP_DEFAULT_RADIUS * 2.0f, LAMP_DEFAULT_RADIUS * 2.0f});
    }
}

void editor_create_lamp(float world_x, float world_y)
{
    ensure_lamps_capacity();
    int snap_x, snap_y;
    snap_to_grid(world_x, world_y, &snap_x, &snap_y);

    EditorLamp *l = &lamps[lamp_count++];
    l->x = (float)snap_x;
    l->y = (float)snap_y;
    l->radius = LAMP_DEFAULT_RADIUS;
    l->logic_lamp = malloc(sizeof(struct Lamp));
    if (l->logic_lamp)
    {
        l->logic_lamp->input = NULL;
        l->logic_lamp->state = UNKNOWN;
    }

    connect_lamp_to_nearby_wire(l);
    lamp_placement_active = false;
}

// Visual gates
static void ensure_gates_capacity(void)
{
    if (gate_count >= gate_capacity)
    {
        gate_capacity = gate_capacity == 0 ? 8 : gate_capacity * 2;
        gates = realloc(gates, gate_capacity * sizeof(EditorGate));
    }
}

// Compute world coordinates for a given gate pin
static void gate_pin_world(const EditorGate *eg, GatePinType pin, float *out_x, float *out_y)
{
    // inputs on the left side, output on the right
    float x = eg->x;
    float y = eg->y;
    float w = eg->width;
    float h = eg->height;
    switch (pin)
    {
    case PIN_INPUT1:
        *out_x = x;
        *out_y = y + h * 0.25f;
        break;
    case PIN_INPUT2:
        *out_x = x;
        *out_y = y + h * 0.75f;
        break;
    case PIN_OUTPUT:
    default:
        *out_x = x + w;
        *out_y = y + h * 0.5f;
        break;
    }
}

// Find nearest gate pin within max_distance; returns 1 if found and fills out gate index and pin
static int find_nearest_gate_pin(float world_x, float world_y, float max_distance, int *out_gate_index, GatePinType *out_pin)
{
    float max_sq = max_distance * max_distance;
    int found = 0;
    float best_sq = max_sq;
    int best_gate = -1;
    GatePinType best_pin = PIN_OUTPUT;
    for (size_t i = 0; i < gate_count; ++i)
    {
        EditorGate *eg = &gates[i];
        for (GatePinType p = PIN_INPUT1; p <= PIN_OUTPUT; p++)
        {
            float px, py;
            gate_pin_world(eg, p, &px, &py);
            float dsq = distance_sq(world_x, world_y, px, py);
            if (dsq <= best_sq)
            {
                best_sq = dsq;
                best_gate = (int)i;
                best_pin = p;
                found = 1;
            }
        }
    }
    if (found)
    {
        *out_gate_index = best_gate;
        *out_pin = best_pin;
        return 1;
    }
    return 0;
}

static int hit_test_gate(float world_x, float world_y)
{
    for (size_t i = 0; i < gate_count; ++i)
    {
        float gx = gates[i].x;
        float gy = gates[i].y;
        float w = gates[i].width;
        float h = gates[i].height;
        if (world_x >= gx && world_x <= gx + w && world_y >= gy && world_y <= gy + h)
            return (int)i;
    }
    return -1;
}

static void detach_gates_from_wire(struct Wire *logic_wire)
{
    if (!logic_wire)
        return;
    for (size_t i = 0; i < gate_count; ++i)
    {
        struct Gate *g = gates[i].gate;
        if (!g)
            continue;
        if (g->input1 == logic_wire)
            g->input1 = NULL;
        if (g->input2 == logic_wire)
            g->input2 = NULL;
        if (g->output == logic_wire)
            g->output = NULL;
    }
}
