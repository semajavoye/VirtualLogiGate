#include "render_utils.h"
#include <string.h>

void render_text_centered(SDL_Renderer *renderer, TTF_Font *font,
                         const char *text, float y, SDL_Color color) {
    SDL_Surface *surface = TTF_RenderText_Blended(font, text, strlen(text), color);
    
    if (surface) {
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        
        if (texture) {
            // Get actual window size
            int window_w, window_h;
            SDL_Window *window = SDL_GetRenderWindow(renderer);
            SDL_GetWindowSizeInPixels(window, &window_w, &window_h);
            
            SDL_FRect dest_rect;
            dest_rect.w = (float)surface->w;
            dest_rect.h = (float)surface->h;
            dest_rect.x = ((float)window_w - dest_rect.w) / 2.0f;
            dest_rect.y = y;
            
            SDL_RenderTexture(renderer, texture, NULL, &dest_rect);
            SDL_DestroyTexture(texture);
        }
        
        SDL_DestroySurface(surface);
    }
}

void render_text(SDL_Renderer *renderer, TTF_Font *font,
                const char *text, float x, float y, SDL_Color color) {
    SDL_Surface *surface = TTF_RenderText_Blended(font, text, strlen(text), color);
    
    if (surface) {
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        
        if (texture) {
            SDL_FRect dest_rect;
            dest_rect.w = (float)surface->w;
            dest_rect.h = (float)surface->h;
            dest_rect.x = x;
            dest_rect.y = y;
            
            SDL_RenderTexture(renderer, texture, NULL, &dest_rect);
            SDL_DestroyTexture(texture);
        }
        
        SDL_DestroySurface(surface);
    }
}