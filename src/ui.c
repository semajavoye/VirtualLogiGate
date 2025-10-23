#include "ui.h"
#include <stdlib.h>
#include <string.h>

UI* ui_create(TTF_Font *font) {
    UI *ui = malloc(sizeof(UI));
    if (!ui) return NULL;
    
    ui->buttons = NULL;
    ui->button_count = 0;
    ui->font = font;
    
    return ui;
}

void ui_destroy(UI *ui) {
    if (ui) {
        if (ui->buttons) {
            free(ui->buttons);
        }
        free(ui);
    }
}

Button* ui_add_button(UI *ui, float x, float y, float w, float h, 
                      const char *text, void (*on_click)(void)) {
    ui->button_count++;
    ui->buttons = realloc(ui->buttons, sizeof(Button) * ui->button_count);
    
    if (!ui->buttons) return NULL;
    
    Button *btn = &ui->buttons[ui->button_count - 1];
    btn->rect.x = x;
    btn->rect.y = y;
    btn->rect.w = w;
    btn->rect.h = h;
    btn->text = text;
    btn->color = (SDL_Color){ 70, 70, 70, 255 };
    btn->hover_color = (SDL_Color){ 100, 100, 100, 255 };
    btn->is_hovered = false;
    btn->on_click = on_click;
    
    return btn;
}

void ui_handle_mouse_motion(UI *ui, float x, float y) {
    for (int i = 0; i < ui->button_count; i++) {
        Button *btn = &ui->buttons[i];
        
        bool inside = (x >= btn->rect.x && x <= btn->rect.x + btn->rect.w &&
                      y >= btn->rect.y && y <= btn->rect.y + btn->rect.h);
        
        btn->is_hovered = inside;
    }
}

void ui_handle_mouse_click(UI *ui, float x, float y) {
    for (int i = 0; i < ui->button_count; i++) {
        Button *btn = &ui->buttons[i];
        
        if (btn->is_hovered && btn->on_click) {
            btn->on_click();
        }
    }
}

void ui_render(UI *ui, SDL_Renderer *renderer) {
    for (int i = 0; i < ui->button_count; i++) {
        Button *btn = &ui->buttons[i];
        
        // Button-Hintergrund
        SDL_Color color = btn->is_hovered ? btn->hover_color : btn->color;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &btn->rect);
        
        // Button-Rand
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        SDL_RenderRect(renderer, &btn->rect);
        
        // Button-Text
        SDL_Color text_color = { 255, 255, 255, 255 };
        SDL_Surface *text_surface = TTF_RenderText_Blended(ui->font, btn->text, 
                                                           strlen(btn->text), text_color);
        
        if (text_surface) {
            SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            
            if (text_texture) {
                SDL_FRect text_rect;
                text_rect.w = (float)text_surface->w;
                text_rect.h = (float)text_surface->h;
                text_rect.x = btn->rect.x + (btn->rect.w - text_rect.w) / 2.0f;
                text_rect.y = btn->rect.y + (btn->rect.h - text_rect.h) / 2.0f;
                
                SDL_RenderTexture(renderer, text_texture, NULL, &text_rect);
                SDL_DestroyTexture(text_texture);
            }
            
            SDL_DestroySurface(text_surface);
        }
    }
}