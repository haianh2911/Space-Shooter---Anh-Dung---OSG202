#include "ui.h"
#include "game.h"
#include <string.h>
#include <stdbool.h>

// Wrapper cho các hàm cũ (tương thích ngược với admin.c, network.c)
bool get_input_string(int row, int col, char *out_str, int max_len, bool is_password, bool allow_letters) {
    (void)row; (void)col;
    return get_sdl_input_string("Nhập dữ liệu:", out_str, max_len, is_password, allow_letters);
}

void show_message(const char *msg) {
    show_sdl_message(msg);
}

// ==========================================
// MENU CHÍNH CỦA GAME
// ==========================================
int show_main_menu() {
    int selected = 0;
    int num_items = 5;
    const char *labels[] = {
        "1. Chơi Game (Offline)",
        "2. Tạo phòng (Máy Chủ)",
        "3. Vào phòng (Người chơi)",
        "4. Quản trị (Admin)",
        "Q. Thoát Game"
    };
    int returns[] = {1, 2, 3, 4, 0};
    
    float title_glow = 0;
    float title_dir = 0.02f;
    
    while (1) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return 0;
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                    case SDLK_w:
                        selected = (selected - 1 + num_items) % num_items;
                        break;
                    case SDLK_DOWN:
                    case SDLK_s:
                        selected = (selected + 1) % num_items;
                        break;
                    case SDLK_RETURN:
                    case SDLK_SPACE:
                        return returns[selected];
                    case SDLK_1: return 1;
                    case SDLK_2: return 2;
                    case SDLK_3: return 3;
                    case SDLK_4: return 4;
                    case SDLK_q: return 0;
                    case SDLK_f: 
                        toggle_fullscreen();
                        break;
                    default: break;
                }
            }
        }
        
        // Animation
        title_glow += title_dir;
        if (title_glow >= 1.0f || title_glow <= 0.0f) title_dir = -title_dir;
        
        clear_screen();
        update_and_draw_stars();
        
        // Title glow animation
        int glow_val = 150 + (int)(title_glow * 105);
        SDL_Color title_color = {(Uint8)glow_val, 255, 255, 255};
        
        // Title
        draw_text_centered("SPACE SHOOTER", 80, FONT_SIZE_LARGE, title_color);
        
        // Subtitle
        draw_text_centered("- Quiz Adventure -", 115, FONT_SIZE_SMALL, COLOR_DIM_WHITE);
        
        // Decorative line
        SDL_SetRenderDrawColor(gfx.renderer, 0, 255, 255, 80);
        SDL_RenderDrawLine(gfx.renderer, 200, 150, 600, 150);
        
        // Menu items
        int btn_w = 380;
        int btn_h = 40;
        int btn_x = (SCREEN_WIDTH - btn_w) / 2;
        int start_y = 180;
        
        for (int i = 0; i < num_items; i++) {
            int btn_y = start_y + i * 55;
            SDL_Color bg = (i == num_items - 1) ? 
                (SDL_Color){80, 30, 30, 200} :  // Nút Thoát màu đỏ sẫm
                (SDL_Color){30, 30, 70, 200};    // Các nút khác
            
            draw_button(labels[i], btn_x, btn_y, btn_w, btn_h,
                       bg, COLOR_WHITE, (i == selected));
            
            // Mũi tên chỉ vị trí
            if (i == selected) {
                draw_text(">", btn_x - 25, btn_y + 10, FONT_SIZE_MEDIUM, COLOR_CYAN);
            }
        }
        
        // Footer hint
        draw_text_centered("[W/S] Di lướt  |  [ENTER] Chọn  |  [F] Toàn màn hình", SCREEN_HEIGHT - 40, 
                          FONT_SIZE_SMALL, COLOR_DIM_WHITE);
        
        present_screen();
        SDL_Delay(FRAME_DELAY);
    }
}

// ==========================================
// MENU DÀNH CHO ADMIN
// ==========================================
int show_admin_menu() {
    int selected = 0;
    int num_items = 7;
    const char *labels[] = {
        "1. Thêm câu hỏi mới",
        "2. Xem danh sách câu hỏi",
        "3. Đổi mật khẩu Admin",
        "4. Đặt lại bộ câu hỏi",
        "5. Import câu hỏi (CSV)",
        "6. Tải file mẫu (CSV)",
        "Q. Quay lại Menu chính"
    };
    int returns[] = {1, 2, 3, 4, 5, 6, 0};
    
    while (1) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return 0;
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                    case SDLK_w:
                        selected = (selected - 1 + num_items) % num_items;
                        break;
                    case SDLK_DOWN:
                    case SDLK_s:
                        selected = (selected + 1) % num_items;
                        break;
                    case SDLK_RETURN:
                    case SDLK_SPACE:
                        return returns[selected];
                    case SDLK_1: return 1;
                    case SDLK_2: return 2;
                    case SDLK_3: return 3;
                    case SDLK_4: return 4;
                    case SDLK_5: return 5;
                    case SDLK_6: return 6;
                    case SDLK_q: return 0;
                    case SDLK_f:
                        toggle_fullscreen();
                        break;
                    default: break;
                }
            }
        }
        
        clear_screen();
        update_and_draw_stars();
        
        // Title
        draw_text_centered("MENU ADMIN", 80, FONT_SIZE_LARGE, COLOR_NEON_ORANGE);
        
        // Line
        SDL_SetRenderDrawColor(gfx.renderer, 255, 165, 0, 80);
        SDL_RenderDrawLine(gfx.renderer, 200, 115, 600, 115);
        
        // Buttons
        int btn_w = 380;
        int btn_h = 36;
        int btn_x = (SCREEN_WIDTH - btn_w) / 2;
        int start_y = 140;
        
        for (int i = 0; i < num_items; i++) {
            int btn_y = start_y + i * 48;
            SDL_Color bg = (i == num_items - 1) ?
                (SDL_Color){60, 30, 30, 200} :
                (SDL_Color){40, 30, 60, 200};
            
            draw_button(labels[i], btn_x, btn_y, btn_w, btn_h,
                       bg, COLOR_WHITE, (i == selected));
            
            if (i == selected) {
                draw_text(">", btn_x - 25, btn_y + 8, FONT_SIZE_MEDIUM, COLOR_NEON_ORANGE);
            }
        }
        
        draw_text_centered("[W/S] Di chuyen  |  [ENTER] Chon", SCREEN_HEIGHT - 40,
                          FONT_SIZE_SMALL, COLOR_DIM_WHITE);
        
        present_screen();
        SDL_Delay(FRAME_DELAY);
    }
}