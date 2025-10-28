#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

void render_text_centered(SDL_Renderer *renderer, TTF_Font *font,
                          const char *text, float y, SDL_Color color);

void render_text(SDL_Renderer *renderer, TTF_Font *font,
                 const char *text, float x, float y, SDL_Color color);

#endif