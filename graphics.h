#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdbool.h>

// ==========================================
// CONSTANTS
// ==========================================
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600
#define FPS 60
#define FRAME_DELAY (1000 / FPS)

// Kích thước game area (phần chơi game)
#define GAME_AREA_X 0
#define GAME_AREA_Y 50
#define GAME_AREA_W SCREEN_WIDTH
#define GAME_AREA_H (SCREEN_HEIGHT - 50)

// Số sao nền
#define MAX_STARS 150
#define STAR_LAYERS 3

// Số particle khi nổ
#define MAX_PARTICLES 30

// Font sizes
#define FONT_SIZE_LARGE  32
#define FONT_SIZE_MEDIUM 24
#define FONT_SIZE_SMALL  18

// ==========================================
// COLORS (SDL_Color)
// ==========================================
#define COLOR_BG          (SDL_Color){10, 10, 30, 255}
#define COLOR_WHITE       (SDL_Color){255, 255, 255, 255}
#define COLOR_CYAN        (SDL_Color){0, 255, 255, 255}
#define COLOR_NEON_GREEN  (SDL_Color){57, 255, 20, 255}
#define COLOR_NEON_ORANGE (SDL_Color){255, 165, 0, 255}
#define COLOR_RED         (SDL_Color){255, 50, 50, 255}
#define COLOR_YELLOW      (SDL_Color){255, 255, 0, 255}
#define COLOR_DARK_GRAY   (SDL_Color){40, 40, 60, 255}
#define COLOR_MID_GRAY    (SDL_Color){80, 80, 120, 255}
#define COLOR_PURPLE      (SDL_Color){150, 50, 255, 255}
#define COLOR_DIM_WHITE   (SDL_Color){150, 150, 170, 255}
#define COLOR_OVERLAY     (SDL_Color){0, 0, 0, 180}

// ==========================================
// STRUCTS
// ==========================================
typedef struct {
    float x, y;
    float speed;
    int brightness;
    int layer; // 0=xa, 1=giữa, 2=gần
} Star;

typedef struct {
    float x, y;
    float vx, vy;
    int life;
    SDL_Color color;
    bool active;
} Particle;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    FT_Library ft_lib;
    FT_Face ft_face;
    Star stars[MAX_STARS];
    Particle particles[MAX_PARTICLES];
    bool initialized;
} GameGraphics;

// ==========================================
// GLOBAL
// ==========================================
extern GameGraphics gfx;

// ==========================================
// CORE FUNCTIONS
// ==========================================
int  init_graphics(void);
void close_graphics(void);
void clear_screen(void);
void present_screen(void);
void toggle_fullscreen(void);

// ==========================================
// DRAWING HELPERS
// ==========================================
// Text (font_size: FONT_SIZE_LARGE, FONT_SIZE_MEDIUM, FONT_SIZE_SMALL)
void draw_text(const char *text, int x, int y, int font_size, SDL_Color color);
void draw_text_centered(const char *text, int y, int font_size, SDL_Color color);
int  draw_text_wrapped(const char *text, int x, int y, int max_w, int line_h, int font_size, SDL_Color color);
int  get_text_width(const char *text, int font_size);

// Shapes
void draw_filled_rect(int x, int y, int w, int h, SDL_Color color);
void draw_rect_outline(int x, int y, int w, int h, SDL_Color color);
void draw_rounded_rect(int x, int y, int w, int h, SDL_Color color);

// Game elements
void init_stars(void);
void update_and_draw_stars(void);
void draw_ship(int x, int y);
void draw_asteroid(int x, int y, float rotation);
void draw_enemy_ship(int x, int y);
void draw_shield_pickup(int x, int y);
void draw_shield_effect(int x, int y, bool blinking);
void draw_health_bar(int x, int y, int lives, int max_lives);
void draw_hud(int score, int lives, int time_remaining);

// Particles
void spawn_particles(int x, int y, SDL_Color color);
void update_and_draw_particles(void);

// UI Components
int  draw_button(const char *text, int x, int y, int w, int h, 
                  SDL_Color bg, SDL_Color text_color, bool selected);
void draw_overlay(void);
void draw_progress_circle(int cx, int cy, int radius, float progress, SDL_Color color);

// Input handling
bool get_sdl_input_string(const char *prompt, char *out_str, int max_len, bool is_password, bool allow_letters);
void show_sdl_message(const char *msg);

// Event helpers
bool poll_quit_event(void);

#endif
