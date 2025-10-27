#include "editor.h"
#include <stdlib.h>
#include <stdbool.h>

// Storage for wire points
typedef struct {
    float x;
    float y;
} WirePoint;

static WirePoint *wire_points = NULL;
static size_t wire_point_count = 0;
static size_t wire_point_capacity = 0;

int rectangle_w = 10; // Width of grid rectangles in pixels
int rectangle_h = 10; // Height of grid rectangles in pixels

int click_count = 0;

void wire_placement_handle_click(SDL_Renderer *renderer, float x, float y) {
    int snapped_y, snapped_x;
    SDL_Log("Wire placement click at (%f, %f)", x, y);
    
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

    // Realign point to grid
    // Get next x, y point
    snapped_x = ((int)(x + 5) / 10) * 10;
    snapped_y = ((int)(y + 5) / 10) * 10;

    // Add the new point
    wire_points[wire_point_count].x = snapped_x;
    wire_points[wire_point_count].y = snapped_y;
    wire_point_count++;
}

void wire_placement_render(SDL_Renderer *renderer) {
    // Render all stored wire points
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    for (size_t i = 0; i < wire_point_count; i++) {
        SDL_RenderPoint(renderer, (int)wire_points[i].x, (int)wire_points[i].y);
    }
}

void wire_placement_clear(void) {
    wire_point_count = 0;
}

// Main editor rendering function
void editor_render(SDL_Renderer *renderer) {
    // Render the grid
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    int w, h;

    SDL_GetCurrentRenderOutputSize(renderer, &w, &h);

    // render lines for the grid
    for (int y = 0; y <= h; y += rectangle_h) {
        SDL_RenderLine(renderer, 0, y, w, y);
    }

    for (int x = 0; x <= w; x += rectangle_w) {
        SDL_RenderLine(renderer, x, 0, x, h);
    }
    
    // Render all gates
    // TODO: Render gates here
    
    // Render all wires
    // TODO: Render wires here
    
    // Render wire points being placed
    wire_placement_render(renderer);
    
    // Render lamps
    // TODO: Render lamps here
}