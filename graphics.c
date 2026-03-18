#include "graphics.h"
#include <SDL.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

GameGraphics gfx;

typedef struct {
    SDL_Texture* texture;
    int advance;
    int bearing_x;
    int bearing_y;
    int width;
    int height;
} Glyph;

#define MAX_GLYPHS 8192

Glyph glyphs_large[MAX_GLYPHS];
Glyph glyphs_medium[MAX_GLYPHS];
Glyph glyphs_small[MAX_GLYPHS];

uint32_t decode_utf8(const char** s) {
    const uint8_t* str = (const uint8_t*)*s;
    uint32_t cp = 0;
    if (str[0] < 0x80) {
        cp = str[0];
        *s += 1;
    } else if ((str[0] & 0xE0) == 0xC0) {
        cp = ((str[0] & 0x1F) << 6) | (str[1] & 0x3F);
        *s += 2;
    } else if ((str[0] & 0xF0) == 0xE0) {
        cp = ((str[0] & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
        *s += 3;
    } else if ((str[0] & 0xF8) == 0xF0) {
        cp = ((str[0] & 0x07) << 18) | ((str[1] & 0x3F) << 12) | ((str[2] & 0x3F) << 6) | (str[3] & 0x3F);
        *s += 4;
    } else {
        cp = '?';
        *s += 1;
    }
    return cp;
}

int ascent_large = 0;
int ascent_medium = 0;
int ascent_small = 0;

void load_font_glyphs(int size, Glyph* glyph_array, int* ascent) {
    FT_Set_Pixel_Sizes(gfx.ft_face, 0, size);
    *ascent = 0;

    for (uint32_t c = 0; c < MAX_GLYPHS; c++) {
        if (c > 127 && FT_Get_Char_Index(gfx.ft_face, c) == 0) continue;
        
        if (FT_Load_Char(gfx.ft_face, c, FT_LOAD_RENDER)) {
            glyph_array[c].texture = NULL;
            continue;
        }
        
        FT_Bitmap* bitmap = &gfx.ft_face->glyph->bitmap;
        if (bitmap->width == 0 || bitmap->rows == 0) {
            glyph_array[c].texture = NULL;
            glyph_array[c].advance = gfx.ft_face->glyph->advance.x >> 6;
            continue;
        }
        
        SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, bitmap->width, bitmap->rows, 32, SDL_PIXELFORMAT_RGBA32);
        
        uint32_t* pixels = (uint32_t*)surface->pixels;
        for (int y = 0; y < bitmap->rows; y++) {
            for (int x = 0; x < bitmap->width; x++) {
                uint8_t alpha = bitmap->buffer[y * bitmap->pitch + x];
                pixels[y * surface->w + x] = (alpha << 24) | 0x00FFFFFF; // white with alpha
            }
        }
        
        glyph_array[c].texture = SDL_CreateTextureFromSurface(gfx.renderer, surface);
        SDL_FreeSurface(surface);
        
        glyph_array[c].width = bitmap->width;
        glyph_array[c].height = bitmap->rows;
        glyph_array[c].bearing_x = gfx.ft_face->glyph->bitmap_left;
        glyph_array[c].bearing_y = gfx.ft_face->glyph->bitmap_top;
        glyph_array[c].advance = gfx.ft_face->glyph->advance.x >> 6;

        if (glyph_array[c].bearing_y > *ascent) {
            *ascent = glyph_array[c].bearing_y;
        }
    }
}

void free_font_glyphs(Glyph* glyph_array) {
    for (int i = 0; i < MAX_GLYPHS; i++) {
        if (glyph_array[i].texture) {
            SDL_DestroyTexture(glyph_array[i].texture);
            glyph_array[i].texture = NULL;
        }
    }
}

int init_graphics() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        printf("SDL Init loi: %s\n", SDL_GetError());
        return -1;
    }

    gfx.window = SDL_CreateWindow("SPACE SHOOTER", 
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                  SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!gfx.window) return -1;

    gfx.renderer = SDL_CreateRenderer(gfx.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!gfx.renderer) return -1;

    SDL_RenderSetLogicalSize(gfx.renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetRenderDrawBlendMode(gfx.renderer, SDL_BLENDMODE_BLEND);

    // Init FreeType
    if (FT_Init_FreeType(&gfx.ft_lib)) {
        printf("FreeType Init loi\n");
        return -1;
    }

    if (FT_New_Face(gfx.ft_lib, "assets/font.ttf", 0, &gfx.ft_face)) {
        printf("Khong the load font assets/font.ttf\n");
        return -1;
    }

    // Load glyphs into memory
    load_font_glyphs(FONT_SIZE_LARGE, glyphs_large, &ascent_large);
    load_font_glyphs(FONT_SIZE_MEDIUM, glyphs_medium, &ascent_medium);
    load_font_glyphs(FONT_SIZE_SMALL, glyphs_small, &ascent_small);

    gfx.initialized = true;
    init_stars();
    return 0;
}

void close_graphics() {
    if (gfx.initialized) {
        free_font_glyphs(glyphs_large);
        free_font_glyphs(glyphs_medium);
        free_font_glyphs(glyphs_small);

        FT_Done_Face(gfx.ft_face);
        FT_Done_FreeType(gfx.ft_lib);

        SDL_DestroyRenderer(gfx.renderer);
        SDL_DestroyWindow(gfx.window);
        SDL_Quit();
        gfx.initialized = false;
    }
}

void clear_screen() {
    SDL_SetRenderDrawColor(gfx.renderer, 10, 10, 20, 255);
    SDL_RenderClear(gfx.renderer);
}

void present_screen() {
    SDL_RenderPresent(gfx.renderer);
}

void toggle_fullscreen() {
    Uint32 flags = SDL_GetWindowFlags(gfx.window);
    if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
        SDL_SetWindowFullscreen(gfx.window, 0);
    } else {
        SDL_SetWindowFullscreen(gfx.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
}

void draw_text(const char *text, int x, int y, int font_size, SDL_Color color) {
    if (!text) return;
    
    Glyph* glyphs;
    int ascent;
    if (font_size == FONT_SIZE_LARGE) { glyphs = glyphs_large; ascent = ascent_large;    }
    else if (font_size == FONT_SIZE_MEDIUM) { glyphs = glyphs_medium; ascent = ascent_medium; }
    else { glyphs = glyphs_small; ascent = ascent_small;   }
    
    int pen_x = x;
    int pen_y = y + ascent; // Baseline
    
    const char *ptr = text;
    while (*ptr) {
        uint32_t c = decode_utf8(&ptr);
        if (c >= MAX_GLYPHS) continue;
        
        Glyph g = glyphs[c];
        if (g.texture) {
            SDL_Rect dest = {
                pen_x + g.bearing_x,
                pen_y - g.bearing_y,
                g.width,
                g.height
            };
            SDL_SetTextureColorMod(g.texture, color.r, color.g, color.b);
            SDL_SetTextureAlphaMod(g.texture, color.a);
            SDL_RenderCopy(gfx.renderer, g.texture, NULL, &dest);
        }
        pen_x += g.advance;
    }
}

int draw_text_wrapped(const char *text, int x, int y, int max_w, int line_h, int font_size, SDL_Color color) {
    if (!text) return 0;
    
    Glyph* glyphs;
    int ascent;
    if (font_size == FONT_SIZE_LARGE) { glyphs = glyphs_large; ascent = ascent_large;    }
    else if (font_size == FONT_SIZE_MEDIUM) { glyphs = glyphs_medium; ascent = ascent_medium; }
    else { glyphs = glyphs_small; ascent = ascent_small;   }
    
    int pen_x = x;
    int pen_y = y + ascent; 
    
    const char *ptr = text;
    while (*ptr) {
        // Look ahead to find word width
        const char *word_end = ptr;
        int word_w = 0;
        while (*word_end && *word_end != ' ' && *word_end != '\n') {
            const char* tmp = word_end;
            uint32_t c = decode_utf8(&tmp);
            if (c < MAX_GLYPHS) word_w += glyphs[c].advance;
            word_end = tmp;
        }
        
        // Wrap condition
        if (pen_x > x && pen_x - x + word_w > max_w) {
            pen_x = x;
            pen_y += line_h;
        }
        
        // Draw the word
        while (ptr < word_end) {
            uint32_t c = decode_utf8(&ptr);
            if (c < MAX_GLYPHS) {
                Glyph g = glyphs[c];
                if (g.texture) {
                    SDL_Rect dest = { pen_x + g.bearing_x, pen_y - g.bearing_y, g.width, g.height };
                    SDL_SetTextureColorMod(g.texture, color.r, color.g, color.b);
                    SDL_SetTextureAlphaMod(g.texture, color.a);
                    SDL_RenderCopy(gfx.renderer, g.texture, NULL, &dest);
                }
                pen_x += g.advance;
            }
        }
        
        // Handle delimiter
        if (*ptr == ' ') {
            uint32_t c = decode_utf8(&ptr);
            if (c < MAX_GLYPHS) pen_x += glyphs[c].advance;
        } else if (*ptr == '\n') {
            ptr++;
            pen_x = x;
            pen_y += line_h;
        }
    }
    return pen_y - y;
}

int get_text_width(const char *text, int font_size) {
    if (!text) return 0;
    Glyph* glyphs;
    if (font_size == FONT_SIZE_LARGE) glyphs = glyphs_large;
    else if (font_size == FONT_SIZE_MEDIUM) glyphs = glyphs_medium;
    else glyphs = glyphs_small;
    
    int w = 0;
    const char *ptr = text;
    while (*ptr) {
        uint32_t c = decode_utf8(&ptr);
        if (c < MAX_GLYPHS) w += glyphs[c].advance;
    }
    return w;
}

void draw_text_centered(const char *text, int y, int font_size, SDL_Color color) {
    int w = get_text_width(text, font_size);
    draw_text(text, (SCREEN_WIDTH - w) / 2, y, font_size, color);
}

void draw_filled_rect(int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(gfx.renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(gfx.renderer, &rect);
}

void draw_rect_outline(int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(gfx.renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderDrawRect(gfx.renderer, &rect);
}

void draw_rounded_rect(int x, int y, int w, int h, SDL_Color color) {
    draw_filled_rect(x, y, w, h, color); // Simple fallback
}

void init_stars() {
    for (int i = 0; i < MAX_STARS; i++) {
        gfx.stars[i].x = rand() % SCREEN_WIDTH;
        gfx.stars[i].y = rand() % SCREEN_HEIGHT;
        gfx.stars[i].layer = rand() % STAR_LAYERS;
        gfx.stars[i].speed = (gfx.stars[i].layer + 1) * 0.5f;
        gfx.stars[i].brightness = 100 + rand() % 155;
    }
}

void update_and_draw_stars() {
    for (int i = 0; i < MAX_STARS; i++) {
        gfx.stars[i].y += gfx.stars[i].speed;
        if (gfx.stars[i].y > SCREEN_HEIGHT) {
            gfx.stars[i].y = 0;
            gfx.stars[i].x = rand() % SCREEN_WIDTH;
        }
        
        uint8_t a = gfx.stars[i].brightness;
        int size = (gfx.stars[i].layer == 2) ? 2 : 1;
        
        SDL_SetRenderDrawColor(gfx.renderer, 255, 255, 255, a);
        SDL_Rect rect = {(int)gfx.stars[i].x, (int)gfx.stars[i].y, size, size};
        SDL_RenderFillRect(gfx.renderer, &rect);
    }
}

void draw_ship(int x, int y) {
    SDL_SetRenderDrawColor(gfx.renderer, 200, 200, 200, 255);
    SDL_Rect core = {x + 10, y + 5, 10, 20};
    SDL_RenderFillRect(gfx.renderer, &core);
    
    SDL_SetRenderDrawColor(gfx.renderer, COLOR_CYAN.r, COLOR_CYAN.g, COLOR_CYAN.b, 255);
    SDL_Rect wing1 = {x, y + 15, 10, 15};
    SDL_Rect wing2 = {x + 20, y + 15, 10, 15};
    SDL_RenderFillRect(gfx.renderer, &wing1);
    SDL_RenderFillRect(gfx.renderer, &wing2);
    
    SDL_SetRenderDrawColor(gfx.renderer, COLOR_NEON_ORANGE.r, COLOR_NEON_ORANGE.g, COLOR_NEON_ORANGE.b, 255);
    SDL_Rect engine = {x + 12, y + 25, 6, 8 + (rand() % 5)};
    SDL_RenderFillRect(gfx.renderer, &engine);
}

void draw_asteroid(int x, int y, float rotation) {
    SDL_SetRenderDrawColor(gfx.renderer, 100, 100, 100, 255);
    SDL_Rect base = {x+5, y, 30, 40};
    SDL_Rect side1 = {x, y+10, 40, 20};
    SDL_RenderFillRect(gfx.renderer, &base);
    SDL_RenderFillRect(gfx.renderer, &side1);
}

void draw_enemy_ship(int x, int y) {
    // Thân tàu địch - màu đỏ
    SDL_SetRenderDrawColor(gfx.renderer, 200, 30, 30, 255);
    SDL_Rect core = {x + 8, y + 5, 14, 20};
    SDL_RenderFillRect(gfx.renderer, &core);
    
    // Cánh tàu địch
    SDL_SetRenderDrawColor(gfx.renderer, 255, 60, 60, 255);
    SDL_Rect wing1 = {x, y + 10, 10, 12};
    SDL_Rect wing2 = {x + 20, y + 10, 10, 12};
    SDL_RenderFillRect(gfx.renderer, &wing1);
    SDL_RenderFillRect(gfx.renderer, &wing2);
    
    // Mũi tàu (phía dưới vì bay xuống)
    SDL_SetRenderDrawColor(gfx.renderer, 255, 100, 0, 255);
    SDL_Rect nose = {x + 11, y + 25, 8, 6};
    SDL_RenderFillRect(gfx.renderer, &nose);
}

void draw_shield_pickup(int x, int y) {
    // Hình khiên - viên kim cương xanh dương
    SDL_SetRenderDrawColor(gfx.renderer, 50, 150, 255, 255);
    SDL_Rect center = {x + 4, y + 2, 12, 16};
    SDL_RenderFillRect(gfx.renderer, &center);
    SDL_Rect top = {x + 6, y, 8, 4};
    SDL_RenderFillRect(gfx.renderer, &top);
    SDL_Rect bot = {x + 6, y + 16, 8, 4};
    SDL_RenderFillRect(gfx.renderer, &bot);
    
    // Viền sáng
    SDL_SetRenderDrawColor(gfx.renderer, 100, 200, 255, 255);
    SDL_Rect outline = {x + 2, y, 16, 20};
    SDL_RenderDrawRect(gfx.renderer, &outline);
}

void draw_shield_effect(int x, int y, bool blinking) {
    if (blinking) {
        Uint32 blink = (SDL_GetTicks() / 150) % 2;
        if (!blink) return; // Nhấp nháy khi sắp hết
    }
    // Vẽ vòng bảo vệ quanh tàu
    SDL_SetRenderDrawColor(gfx.renderer, 50, 150, 255, 180);
    SDL_Rect ring = {x - 5, y - 5, 40, 45};
    SDL_RenderDrawRect(gfx.renderer, &ring);
    SDL_Rect ring2 = {x - 3, y - 3, 36, 41};
    SDL_RenderDrawRect(gfx.renderer, &ring2);
}

void draw_health_bar(int x, int y, int lives, int max_lives) {
    for (int i = 0; i < max_lives; i++) {
        SDL_Color color = (i < lives) ? COLOR_RED : COLOR_DARK_GRAY;
        draw_filled_rect(x + i*25, y, 20, 20, color);
    }
}

void draw_hud(int score, int lives, int time_remaining) {
    draw_filled_rect(0, 0, SCREEN_WIDTH, 50, (SDL_Color){20, 20, 30, 255});
    draw_rect_outline(0, 49, SCREEN_WIDTH, 1, COLOR_CYAN);
    
    char score_str[32];
    sprintf(score_str, "ĐIỂM: %d", score);
    draw_text(score_str, 20, 15, FONT_SIZE_MEDIUM, COLOR_NEON_GREEN);
    
    draw_health_bar(200, 15, lives, 3);
    
    if (time_remaining >= 0) {
        char time_str[32];
        sprintf(time_str, "THỜI GIAN: %2d", time_remaining);
        SDL_Color timer_color = (time_remaining <= 5) ? COLOR_RED : COLOR_YELLOW;
        draw_text(time_str, SCREEN_WIDTH - 250, 15, FONT_SIZE_MEDIUM, timer_color);
    }
}

void spawn_particles(int x, int y, SDL_Color color) {
    int spawned = 0;
    for (int i = 0; i < MAX_PARTICLES && spawned < 15; i++) {
        if (!gfx.particles[i].active) {
            gfx.particles[i].active = true;
            gfx.particles[i].x = x;
            gfx.particles[i].y = y;
            float angle = (rand() % 360) * 3.14159f / 180.0f;
            float speed = 1.0f + (rand() % 40) / 10.0f;
            gfx.particles[i].vx = cos(angle) * speed;
            gfx.particles[i].vy = sin(angle) * speed;
            gfx.particles[i].life = 20 + rand() % 20;
            gfx.particles[i].color = color;
            spawned++;
        }
    }
}

void update_and_draw_particles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (gfx.particles[i].active) {
            gfx.particles[i].x += gfx.particles[i].vx;
            gfx.particles[i].y += gfx.particles[i].vy;
            gfx.particles[i].life--;
            
            if (gfx.particles[i].life <= 0) {
                gfx.particles[i].active = false;
            } else {
                uint8_t a = (gfx.particles[i].life * 255) / 40;
                SDL_SetRenderDrawColor(gfx.renderer, gfx.particles[i].color.r, 
                                     gfx.particles[i].color.g, gfx.particles[i].color.b, a);
                SDL_Rect p = {(int)gfx.particles[i].x, (int)gfx.particles[i].y, 3, 3};
                SDL_RenderFillRect(gfx.renderer, &p);
            }
        }
    }
}

int draw_button(const char *text, int x, int y, int w, int h, 
                 SDL_Color bg, SDL_Color text_color, bool selected) {
    if (selected) {
        bg.r = (bg.r + 50 > 255) ? 255 : bg.r + 50;
        bg.g = (bg.g + 50 > 255) ? 255 : bg.g + 50;
        bg.b = (bg.b + 50 > 255) ? 255 : bg.b + 50;
        
        draw_rect_outline(x-2, y-2, w+4, h+4, COLOR_CYAN);
    }
    
    draw_filled_rect(x, y, w, h, bg);
    draw_rect_outline(x, y, w, h, COLOR_MID_GRAY);
    
    int text_w = get_text_width(text, FONT_SIZE_MEDIUM);
    draw_text(text, x + (w - text_w) / 2, y + (h - 30) / 2, FONT_SIZE_MEDIUM, text_color);
    
    return 0; // SDL2 mouse interaction removed for simplicity, using keyboard
}

void draw_overlay() {
    draw_filled_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_OVERLAY);
}

void draw_progress_circle(int cx, int cy, int radius, float progress, SDL_Color color) {
    draw_filled_rect(cx - radius, cy - radius, radius * 2, radius * 2, COLOR_DARK_GRAY);
    draw_filled_rect(cx - radius, cy - radius, radius * 2 * progress, radius * 2, color);
}

bool poll_quit_event() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) return true;
    }
    return false;
}

bool get_sdl_input_string(const char *prompt, char *out_str, int max_len, bool is_password, bool allow_letters) {
    out_str[0] = '\0';
    int len = 0;
    bool done = false;
    char composition[32] = "";
    
    SDL_Event flush_e;
    while (SDL_PollEvent(&flush_e)); // Flush event queue to prevent phantom inputs from previous menus
    
    SDL_StartTextInput();
    while (!done) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) { done = true; break; }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
                    done = true;
                } else if (e.key.keysym.sym == SDLK_ESCAPE || 
                         (e.key.keysym.sym == SDLK_q && len == 0 && strlen(composition) == 0) ) { 
                    SDL_StopTextInput();
                    return false; // User cancelled
                } else if (e.key.keysym.sym == SDLK_BACKSPACE && len > 0 && strlen(composition) == 0) {
                    do { out_str[--len] = '\0'; } 
                    while(len > 0 && (out_str[len] & 0xC0) == 0x80);
                }
            }
            if (e.type == SDL_TEXTEDITING) {
                strcpy(composition, e.edit.text);
            }
            if (e.type == SDL_TEXTINPUT) {
                composition[0] = '\0';
                if (len + strlen(e.text.text) < max_len - 1) {
                    char c = e.text.text[0]; // Lấy ký tự nhập
                    bool is_valid = true;
                    if (!allow_letters) {
                        // Chỉ cho phép số và dấu chấm (dành cho IP)
                        if ((c < '0' || c > '9') && c != '.') {
                            is_valid = false;
                        }
                    }
                    
                    if (is_valid) {
                        strcat(out_str, e.text.text);
                        len += strlen(e.text.text);
                    }
                }
            }
        }
        
        clear_screen();
        update_and_draw_stars();
        draw_overlay();
        
        draw_filled_rect(100, 200, 600, 150, (SDL_Color){20, 20, 50, 255});
        draw_rect_outline(100, 200, 600, 150, COLOR_CYAN);
        
        draw_text_centered(prompt, 230, FONT_SIZE_MEDIUM, COLOR_YELLOW);
        
        char display_str[256] = "";
        char combined[300] = "";
        if (is_password) {
            for (int i=0; i<len + (int)strlen(composition); i++) display_str[i] = '*';
            display_str[len + strlen(composition)] = '\0';
            strcpy(combined, display_str);
        } else {
            strcpy(display_str, out_str);
            sprintf(combined, "%s%s", display_str, composition);
        }
        
        strcat(combined, "_");
        draw_text_centered(combined, 280, FONT_SIZE_LARGE, COLOR_WHITE);
        
        present_screen(); // Fix màn hình bị delay input vì thieu present màn hình
        SDL_Delay(FRAME_DELAY);
    }
    SDL_StopTextInput();
    return true; // Successfully entered
}

void show_sdl_message(const char *msg) {
    bool done = false;
    while (!done) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) done = true;
            if (e.type == SDL_KEYDOWN && (e.key.keysym.sym == SDLK_SPACE || e.key.keysym.sym == SDLK_RETURN)) {
                done = true;
            }
        }
        
        clear_screen();
        update_and_draw_stars();
        
        int msg_w = get_text_width(msg, FONT_SIZE_MEDIUM);
        int box_w = msg_w + 60;
        if (box_w < 500) box_w = 500;
        if (box_w > SCREEN_WIDTH - 40) box_w = SCREEN_WIDTH - 40;
        int box_x = (SCREEN_WIDTH - box_w) / 2;

        draw_filled_rect(box_x, 250, box_w, 100, (SDL_Color){30, 10, 10, 255});
        draw_rect_outline(box_x, 250, box_w, 100, COLOR_RED);
        
        draw_text_centered(msg, 280, FONT_SIZE_MEDIUM, COLOR_WHITE);
        draw_text_centered("[ SPACE ] để tiếp tục", 320, FONT_SIZE_SMALL, COLOR_DIM_WHITE);
        
        present_screen();
        SDL_Delay(FRAME_DELAY);
    }
}
