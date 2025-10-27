#ifndef CAMERA_H
#define CAMERA_H

#include <stdbool.h>

/**
 * Camera struct for managing view transformations in the circuit editor.
 * Handles panning (offset) and zooming (scaling) of the world space.
 */
typedef struct Camera {
    float x;           // Camera position in world space (horizontal offset)
    float y;           // Camera position in world space (vertical offset)
    float zoom;        // Zoom factor (1.0 = normal, >1.0 = zoomed in, <1.0 = zoomed out)
    
    // Zoom constraints
    float min_zoom;    // Minimum zoom level (e.g., 0.25)
    float max_zoom;    // Maximum zoom level (e.g., 4.0)
    
    // Panning state for smooth dragging
    bool is_panning;
    float pan_start_x;
    float pan_start_y;
    float pan_start_cam_x;
    float pan_start_cam_y;
} Camera;

/**
 * Initialize a camera with default values.
 * Default: centered at origin, zoom 1.0, zoom range [0.25, 4.0].
 * 
 * @param camera Pointer to the camera to initialize
 */
void camera_init(Camera *camera);

/**
 * Apply zoom to the camera, centered around a pivot point (e.g., mouse cursor).
 * This ensures the point under the cursor stays in place during zoom.
 * 
 * @param camera Pointer to the camera
 * @param zoom_delta The amount to zoom (positive = zoom in, negative = zoom out)
 * @param pivot_screen_x Screen X coordinate of the zoom pivot point
 * @param pivot_screen_y Screen Y coordinate of the zoom pivot point
 */
void camera_zoom(Camera *camera, float zoom_delta, float pivot_screen_x, float pivot_screen_y);

/**
 * Start panning the camera (e.g., when middle mouse button is pressed).
 * 
 * @param camera Pointer to the camera
 * @param screen_x Current mouse X position in screen space
 * @param screen_y Current mouse Y position in screen space
 */
void camera_start_pan(Camera *camera, float screen_x, float screen_y);

/**
 * Update camera pan based on current mouse position during drag.
 * Panning speed is consistent across zoom levels.
 * 
 * @param camera Pointer to the camera
 * @param screen_x Current mouse X position in screen space
 * @param screen_y Current mouse Y position in screen space
 */
void camera_update_pan(Camera *camera, float screen_x, float screen_y);

/**
 * Stop panning the camera (e.g., when middle mouse button is released).
 * 
 * @param camera Pointer to the camera
 */
void camera_stop_pan(Camera *camera);

/**
 * Convert world coordinates to screen coordinates using camera transformation.
 * 
 * @param camera Pointer to the camera
 * @param world_x X coordinate in world space
 * @param world_y Y coordinate in world space
 * @param screen_x Output pointer for screen X coordinate
 * @param screen_y Output pointer for screen Y coordinate
 */
void camera_world_to_screen(const Camera *camera, float world_x, float world_y, 
                           float *screen_x, float *screen_y);

/**
 * Convert screen coordinates to world coordinates using camera transformation.
 * Used for converting mouse input to world positions.
 * 
 * @param camera Pointer to the camera
 * @param screen_x X coordinate in screen space
 * @param screen_y Y coordinate in screen space
 * @param world_x Output pointer for world X coordinate
 * @param world_y Output pointer for world Y coordinate
 */
void camera_screen_to_world(const Camera *camera, float screen_x, float screen_y, 
                           float *world_x, float *world_y);

/**
 * Get the current zoom level of the camera.
 * 
 * @param camera Pointer to the camera
 * @return Current zoom factor
 */
float camera_get_zoom(const Camera *camera);

/**
 * Set the camera position in world space.
 * 
 * @param camera Pointer to the camera
 * @param x X position in world space
 * @param y Y position in world space
 */
void camera_set_position(Camera *camera, float x, float y);

#endif // CAMERA_H
