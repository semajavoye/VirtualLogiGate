#include "camera.h"
#include <math.h>

void camera_init(Camera *camera) {
    camera->x = 0.0f;
    camera->y = 0.0f;
    camera->zoom = 1.0f;
    camera->min_zoom = 0.25f;
    camera->max_zoom = 4.0f;
    camera->is_panning = false;
    camera->pan_start_x = 0.0f;
    camera->pan_start_y = 0.0f;
    camera->pan_start_cam_x = 0.0f;
    camera->pan_start_cam_y = 0.0f;
}

void camera_zoom(Camera *camera, float zoom_delta, float pivot_screen_x, float pivot_screen_y) {
    // Get world position before zoom
    float world_x_before, world_y_before;
    camera_screen_to_world(camera, pivot_screen_x, pivot_screen_y, 
                          &world_x_before, &world_y_before);
    
    // Apply zoom with exponential scaling for smooth feel
    float zoom_factor = 1.0f + zoom_delta * 0.1f;
    camera->zoom *= zoom_factor;
    
    // Clamp zoom to allowed range
    if (camera->zoom < camera->min_zoom) {
        camera->zoom = camera->min_zoom;
    }
    if (camera->zoom > camera->max_zoom) {
        camera->zoom = camera->max_zoom;
    }
    
    // Get world position after zoom
    float world_x_after, world_y_after;
    camera_screen_to_world(camera, pivot_screen_x, pivot_screen_y, 
                          &world_x_after, &world_y_after);
    
    // Adjust camera position to keep the pivot point stationary
    camera->x += (world_x_before - world_x_after);
    camera->y += (world_y_before - world_y_after);
}

void camera_start_pan(Camera *camera, float screen_x, float screen_y) {
    camera->is_panning = true;
    camera->pan_start_x = screen_x;
    camera->pan_start_y = screen_y;
    camera->pan_start_cam_x = camera->x;
    camera->pan_start_cam_y = camera->y;
}

void camera_update_pan(Camera *camera, float screen_x, float screen_y) {
    if (!camera->is_panning) {
        return;
    }
    
    // Calculate screen-space delta
    float delta_x = screen_x - camera->pan_start_x;
    float delta_y = screen_y - camera->pan_start_y;
    
    // Convert screen delta to world delta (inversely proportional to zoom)
    // This ensures consistent panning speed across zoom levels
    float world_delta_x = delta_x / camera->zoom;
    float world_delta_y = delta_y / camera->zoom;
    
    // Update camera position (subtract because we're moving the world, not the view)
    camera->x = camera->pan_start_cam_x - world_delta_x;
    camera->y = camera->pan_start_cam_y - world_delta_y;
}

void camera_stop_pan(Camera *camera) {
    camera->is_panning = false;
}

void camera_world_to_screen(const Camera *camera, float world_x, float world_y, 
                           float *screen_x, float *screen_y) {
    // Transform: screen = (world - camera_offset) * zoom
    *screen_x = (world_x - camera->x) * camera->zoom;
    *screen_y = (world_y - camera->y) * camera->zoom;
}

void camera_screen_to_world(const Camera *camera, float screen_x, float screen_y, 
                           float *world_x, float *world_y) {
    // Inverse transform: world = screen / zoom + camera_offset
    *world_x = screen_x / camera->zoom + camera->x;
    *world_y = screen_y / camera->zoom + camera->y;
}

float camera_get_zoom(const Camera *camera) {
    return camera->zoom;
}

void camera_set_position(Camera *camera, float x, float y) {
    camera->x = x;
    camera->y = y;
}
