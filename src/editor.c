#include "editor.h"
#include "camera.h"
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <stdbool.h>

// Global camera instance for the editor
static Camera editor_camera;

// Storage for wire points (in world coordinates)
typedef struct {
    float x;
    float y;
} WirePoint;

static WirePoint *wire_points = NULL;
static size_t wire_point_count = 0;
static size_t wire_point_capacity = 0;

// Store multiple wires
typedef struct {
    WirePoint *points; // dynamic array of points for this wire
    size_t count;
    size_t capacity;
} Wire;

static Wire *wires = NULL;
static size_t wire_count = 0;
static size_t wire_capacity = 0;

// Index of selected wire (-1 = none)
static int selected_wire = -1;

// Wire placement state
static bool wire_active = false;          // Whether a wire is currently being placed
static float pointer_world_x = 0.0f;     // Current pointer position in world coords (for preview)
static float pointer_world_y = 0.0f;

int rectangle_w = 10; // Width of grid rectangles in pixels (world space)
int rectangle_h = 10; // Height of grid rectangles in pixels (world space)

// int click_count = 0; // not needed with stateful implementation

void editor_init(void) {
    camera_init(&editor_camera);
}

Camera* editor_get_camera(void) {
    return &editor_camera;
}

// Internal helper to ensure capacity
static void ensure_wire_capacity(void) {
    if (wire_point_count >= wire_point_capacity) {
        wire_point_capacity = wire_point_capacity == 0 ? 16 : wire_point_capacity * 2;
        wire_points = realloc(wire_points, wire_point_capacity * sizeof(WirePoint));
    }
}

static void ensure_wires_capacity(void) {
    if (wire_count >= wire_capacity) {
        wire_capacity = wire_capacity == 0 ? 8 : wire_capacity * 2;
        wires = realloc(wires, wire_capacity * sizeof(Wire));
    }
}

// Snap world position to grid
static void snap_to_grid(float world_x, float world_y, int *out_x, int *out_y) {
    *out_x = ((int)(world_x + rectangle_w / 2) / rectangle_w) * rectangle_w;
    *out_y = ((int)(world_y + rectangle_h / 2) / rectangle_h) * rectangle_h;
}

// Start a new wire
void wire_placement_start(float world_x, float world_y) {
    wire_placement_clear();
    wire_active = true;
    wire_placement_update_pointer(world_x, world_y);
    // add first point
    wire_placement_add_point(world_x, world_y);
}

// Add a new bend point
void wire_placement_add_point(float world_x, float world_y) {
    if (!wire_active) {
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
void wire_placement_finish(void) {
    if (!wire_active) return;
    wire_active = false;
    // Convert current wire_points array into a Wire and store it
    if (wire_point_count > 0) {
        ensure_wires_capacity();
        Wire *w = &wires[wire_count];
        w->count = wire_point_count;
        w->capacity = wire_point_count;
        w->points = malloc(sizeof(WirePoint) * w->capacity);
        for (size_t i = 0; i < wire_point_count; ++i) {
            w->points[i] = wire_points[i];
        }
        wire_count++;
    }
    // clear temporary placement buffer but keep stored wires
    if (wire_points) {
        free(wire_points);
        wire_points = NULL;
    }
    wire_point_count = 0;
    wire_point_capacity = 0;
}

// Cancel current placement and discard points
void wire_placement_cancel(void) {
    wire_active = false;
    wire_placement_clear();
}

// Update pointer for live preview (world coords)
void wire_placement_update_pointer(float world_x, float world_y) {
    pointer_world_x = world_x;
    pointer_world_y = world_y;
}

int wire_placement_is_active(void) {
    return wire_active ? 1 : 0;
}

void wire_placement_render(SDL_Renderer *renderer, const Camera *camera) {
    // Render all stored wire points and preview segments
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Draw points (small squares) and connecting lines
    for (size_t i = 0; i < wire_point_count; i++) {
        float sx, sy;
        camera_world_to_screen(camera, wire_points[i].x, wire_points[i].y, &sx, &sy);
        // Draw small 3x3 square for visibility
        SDL_RenderFillRect(renderer, &(SDL_FRect){ sx - 1.5f, sy - 1.5f, 3.0f, 3.0f });

        // Draw line to next point
        if (i + 1 < wire_point_count) {
            float nx, ny;
            camera_world_to_screen(camera, wire_points[i+1].x, wire_points[i+1].y, &nx, &ny);
            SDL_RenderLine(renderer, sx, sy, nx, ny);
        }
    }

    // If currently placing a wire and there is at least one point, draw preview segment
    if (wire_active && wire_point_count > 0) {
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
        SDL_RenderFillRect(renderer, &(SDL_FRect){ px - 1.5f, py - 1.5f, 3.0f, 3.0f });
        // reset color
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    }
}

void wire_placement_clear(void) {
    wire_point_count = 0;
    wire_active = false;
    if (wire_points) {
        free(wire_points);
        wire_points = NULL;
    }
    wire_point_capacity = 0;
}

// Helper: free all stored wires
static void free_all_wires(void) {
    for (size_t i = 0; i < wire_count; ++i) {
        free(wires[i].points);
    }
    free(wires);
    wires = NULL;
    wire_count = 0;
    wire_capacity = 0;
    selected_wire = -1;
}

void editor_shutdown(void) {
    wire_placement_clear();
    free_all_wires();
}

// Compute squared distance from point to segment (ax,ay)-(bx,by)
static float point_segment_distance_sq(float px, float py, float ax, float ay, float bx, float by) {
    float vx = bx - ax;
    float vy = by - ay;
    float wx = px - ax;
    float wy = py - ay;
    float c1 = vx * wx + vy * wy;
    if (c1 <= 0) return wx*wx + wy*wy;
    float c2 = vx*vx + vy*vy;
    if (c2 <= c1) {
        float dx = px - bx;
        float dy = py - by;
        return dx*dx + dy*dy;
    }
    float b = c1 / c2;
    float projx = ax + b * vx;
    float projy = ay + b * vy;
    float dx = px - projx;
    float dy = py - projy;
    return dx*dx + dy*dy;
}

// Hit-test wires: returns index of wire hit or -1
static int hit_test_wire(float world_x, float world_y) {
    const float pick_radius = 8.0f; // world-space tolerance
    float pick_sq = pick_radius * pick_radius;
    for (size_t i = 0; i < wire_count; ++i) {
        Wire *w = &wires[i];
        if (w->count < 2) continue;
        for (size_t s = 0; s + 1 < w->count; ++s) {
            if (point_segment_distance_sq(world_x, world_y, w->points[s].x, w->points[s].y, w->points[s+1].x, w->points[s+1].y) <= pick_sq) {
                return (int)i;
            }
        }
    }
    return -1;
}

int editor_select_wire_at(float world_x, float world_y, const Camera *camera) {
    (void)camera; // camera unused here because hit-testing is done in world coords
    int idx = hit_test_wire(world_x, world_y);
    if (idx >= 0) {
        selected_wire = idx;
        return 1;
    }
    selected_wire = -1;
    return 0;
}

void editor_delete_selected(void) {
    if (selected_wire < 0 || (size_t)selected_wire >= wire_count) return;
    // free the selected wire's points
    free(wires[selected_wire].points);
    // shift remaining wires down
    for (size_t i = selected_wire; i + 1 < wire_count; ++i) {
        wires[i] = wires[i+1];
    }
    wire_count--;
    selected_wire = -1;
}

// Main editor rendering function
void editor_render(SDL_Renderer *renderer) {
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
    for (int world_y = grid_start_y; world_y <= grid_end_y; world_y += rectangle_h) {
        float screen_y_start, screen_y_end, screen_x_start, screen_x_end;
        camera_world_to_screen(&editor_camera, world_left, world_y, &screen_x_start, &screen_y_start);
        camera_world_to_screen(&editor_camera, world_right, world_y, &screen_x_end, &screen_y_end);
        SDL_RenderLine(renderer, screen_x_start, screen_y_start, screen_x_end, screen_y_end);
    }

    // Render vertical grid lines
    for (int world_x = grid_start_x; world_x <= grid_end_x; world_x += rectangle_w) {
        float screen_x_start, screen_x_end, screen_y_start, screen_y_end;
        camera_world_to_screen(&editor_camera, world_x, world_top, &screen_x_start, &screen_y_start);
        camera_world_to_screen(&editor_camera, world_x, world_bottom, &screen_x_end, &screen_y_end);
        SDL_RenderLine(renderer, screen_x_start, screen_y_start, screen_x_end, screen_y_end);
    }
    
    // Render all gates
    // TODO: Render gates here
    
    // Render all wires
    // Render stored wires
    for (size_t i = 0; i < wire_count; ++i) {
        Wire *w = &wires[i];
        if (w->count < 1) continue;
        // pick color: selected wires highlighted
        if ((int)i == selected_wire) {
            SDL_SetRenderDrawColor(renderer, 255, 130, 130, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
        }
        for (size_t s = 0; s < w->count; ++s) {
            float sx, sy;
            camera_world_to_screen(&editor_camera, w->points[s].x, w->points[s].y, &sx, &sy);
            // draw point marker
            SDL_RenderFillRect(renderer, &(SDL_FRect){ sx - 1.5f, sy - 1.5f, 3.0f, 3.0f });
            if (s + 1 < w->count) {
                float nx, ny;
                camera_world_to_screen(&editor_camera, w->points[s+1].x, w->points[s+1].y, &nx, &ny);
                SDL_RenderLine(renderer, sx, sy, nx, ny);
            }
        }
    }
    
    // Render wire points being placed
    wire_placement_render(renderer, &editor_camera);
    
    // Render lamps
    // TODO: Render lamps here
}