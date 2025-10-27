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

int rectangle_w = 10; // Width of grid rectangles in pixels (world space)
int rectangle_h = 10; // Height of grid rectangles in pixels (world space)

int click_count = 0;

void editor_init(void) {
    camera_init(&editor_camera);
}

Camera* editor_get_camera(void) {
    return &editor_camera;
}

void wire_placement_handle_click(float world_x, float world_y) {
    int snapped_x, snapped_y;
    SDL_Log("Wire placement click at world (%f, %f)", world_x, world_y);
    
    if (click_count < 1) {
        click_count = click_count + 1;
        SDL_Log("%d", click_count);
    } else {
        click_count = 0;
        SDL_Log("%d", click_count);
    }

    // Expand capacity if needed
    if (wire_point_count >= wire_point_capacity) {
        wire_point_capacity = wire_point_capacity == 0 ? 16 : wire_point_capacity * 2;
        wire_points = realloc(wire_points, wire_point_capacity * sizeof(WirePoint));
    }

    // Snap to grid (10px grid in world space)
    snapped_x = ((int)(world_x + 5) / 10) * 10;
    snapped_y = ((int)(world_y + 5) / 10) * 10;

    // Add the new point (stored in world coordinates)
    wire_points[wire_point_count].x = snapped_x;
    wire_points[wire_point_count].y = snapped_y;
    wire_point_count++;
}

void wire_placement_render(SDL_Renderer *renderer, const Camera *camera) {
    // Render all stored wire points
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    for (size_t i = 0; i < wire_point_count; i++) {
        // Convert world coordinates to screen coordinates
        float screen_x, screen_y;
        camera_world_to_screen(camera, wire_points[i].x, wire_points[i].y, 
                              &screen_x, &screen_y);
        SDL_RenderPoint(renderer, (int)screen_x, (int)screen_y);
    }
}

void wire_placement_clear(void) {
    wire_point_count = 0;
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
    // TODO: Render wires here
    
    // Render wire points being placed
    wire_placement_render(renderer, &editor_camera);
    
    // Render lamps
    // TODO: Render lamps here
}