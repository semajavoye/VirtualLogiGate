#ifndef UI_H
#define UI_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdbool.h>

typedef struct {
    SDL_FRect rect;
    const char *text;
    SDL_Color color;
    SDL_Color hover_color;
    bool is_hovered;
    void (*on_click)(void);
} Button;

typedef struct {
    Button *buttons;
    int button_count;
    TTF_Font *font;
} UI;

typedef enum {
    UI_STATE_MAIN_MENU,
    UI_STATE_INGAME
} AppUIState;

UI* ui_create(TTF_Font *font);
void ui_destroy(UI *ui);

Button* ui_add_button(UI *ui, float x, float y, float w, float h, 
                      const char *text, void (*on_click)(void));

void ui_handle_mouse_motion(UI *ui, float x, float y);
void ui_handle_mouse_click(UI *ui, float x, float y);
void ui_render(UI *ui, SDL_Renderer *renderer);

#endif