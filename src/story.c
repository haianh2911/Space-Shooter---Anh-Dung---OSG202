#include "game.h"
#include "ui.h"
#include "graphics.h"
#include "story.h"
#include <string.h>
#include <stdbool.h>

// ==========================================
// CỐT TRUYỆN (Typewriter effect)
// ==========================================
static int utf8_strlen(const char *str) {
    int len = 0;
    while (*str) {
        if ((*str & 0xC0) != 0x80) len++;
        str++;
    }
    return len;
}

static int get_utf8_byte_len(const char *str, int num_chars) {
    int bytes = 0;
    int chars = 0;
    while (str[bytes] != '\0' && chars < num_chars) {
        if ((str[bytes] & 0xC0) != 0x80) chars++;
        if (chars > num_chars) break;
        bytes++;
    }
    return bytes;
}

void showIntroSequence() {
    // 1. Logo chớp qua
    Uint32 start_time = SDL_GetTicks();
    while (SDL_GetTicks() - start_time < 3500) { // 3.5 seconds
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) return;
        }
        
        clear_screen();
        update_and_draw_stars();
        
        Uint32 blink = (SDL_GetTicks() / 200) % 2;
        if (blink) {
            draw_text_centered("SPACE SHOOTER", SCREEN_HEIGHT / 2 - 20, FONT_SIZE_LARGE, COLOR_CYAN);
        }
        
        present_screen();
        SDL_Delay(FRAME_DELAY);
    }
    
    // 2. Màn hình "Bạn đã nhận được một tin nhắn mới"
    start_time = SDL_GetTicks();
    while (SDL_GetTicks() - start_time < 3500) { // 3.5 seconds
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) return;
        }
        
        clear_screen();
        update_and_draw_stars();
        
        draw_text_centered("Bạn đã nhận được một tin nhắn mới...", SCREEN_HEIGHT / 2 - 20, FONT_SIZE_MEDIUM, COLOR_YELLOW);
        
        present_screen();
        SDL_Delay(FRAME_DELAY);
    }
    
    // 3. Chạy cốt truyện
    showStory();
}

void showStory() {
    const char *story_text = "Năm 2250, Trái Đất cạn kiệt tài nguyên hoàn toàn. Hy vọng cuối cùng của nhân loại đặt lên vai bạn - điều khiển phi thuyền thám hiểm tiến vào vùng không gian sâu thẳm. Nguồn tài nguyên quý giá đang bị phong ấn bên trong lõi của các tiểu hành tinh cổ đại. Để khai thác, yêu cầu bạn phải giải mã các \"khóa tri thức\". Mỗi khi bạn trả lời đúng một câu hỏi, lớp vỏ tiểu hành tinh sẽ vỡ vụn, giải phóng năng lượng và khoáng sản thiết yếu để gửi về cứu Trái Đất. Tuy nhiên, hành trình này đầy rẫy hiểm nguy. Bạn không chỉ phải tập trung giải đố mà còn phải khéo léo lèo lái phi thuyền né tránh các tàu từ thế lực thù địch đang chực chờ cướp nguồn năng lượng của bạn. Trí tuệ và sự phản xạ của bạn chính là hy vọng duy nhất của nhân loại!";
    
    int total_chars = utf8_strlen(story_text);
    int chars_shown = 0;
    bool done = false;
    bool skipped = false;
    Uint32 last_char_time = SDL_GetTicks();
    int char_delay = 60; // ms giữa mỗi ký tự

    while (!done) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return;
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_SPACE) {
                    if (skipped || chars_shown >= total_chars) {
                        done = true; // Tiếp tục
                    } else {
                        // Skip - hiện tất cả
                        skipped = true;
                        chars_shown = total_chars;
                    }
                }
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    done = true;
                }
            }
        }

        // Typewriter logic
        if (!skipped && chars_shown < total_chars) {
            if (SDL_GetTicks() - last_char_time > (Uint32)char_delay) {
                chars_shown++;
                last_char_time = SDL_GetTicks();
            }
        }

        // Render
        clear_screen();
        update_and_draw_stars();

        // Title
        draw_text_centered("CỐT TRUYỆN", 40, FONT_SIZE_LARGE, COLOR_CYAN);

        // Story text
        if (chars_shown > 0) {
            char partial[4096];
            int byte_len = get_utf8_byte_len(story_text, chars_shown);
            strncpy(partial, story_text, byte_len);
            partial[byte_len] = '\0';
            
            // Render văn bản liền mạch tự động Word Wrap (giảm khoảng cách dòng xuống 28)
            draw_text_wrapped(partial, 80, 100, 640, 28, FONT_SIZE_SMALL, COLOR_DIM_WHITE);
        }

        // Button hint
        if (skipped || chars_shown >= total_chars) {
            // Nhấp nháy
            Uint32 blink = (SDL_GetTicks() / 500) % 2;
            if (blink) {
                draw_text_centered("[ SPACE: Bắt đầu chơi! ]", SCREEN_HEIGHT - 50,
                                  FONT_SIZE_SMALL, COLOR_CYAN);
            }
        } else {
            draw_text_centered("[ SPACE: Bỏ qua ]", SCREEN_HEIGHT - 50,
                              FONT_SIZE_SMALL, COLOR_DIM_WHITE);
        }

        present_screen();
        SDL_Delay(FRAME_DELAY);
    }
}

// ==========================================
// HƯỚNG DẪN CHƠI
// ==========================================
void showInstructions() {
    bool done = false;

    while (!done) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return;
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_SPACE ||
                    e.key.keysym.sym == SDLK_RETURN) {
                    done = true;
                }
            }
        }

        clear_screen();
        update_and_draw_stars();

        // Title
        draw_text_centered("HƯỚNG DẪN CHƠI", 25, FONT_SIZE_LARGE, COLOR_YELLOW);

        // === CỘT TRÁI: Luật chơi ===
        int lx = 30, ly = 70;
        int box_w = 400, box_h = 440;
        draw_filled_rect(lx, ly, box_w, box_h, (SDL_Color){15, 15, 40, 220});
        draw_rect_outline(lx, ly, box_w, box_h, COLOR_YELLOW);

        int y = ly + 15;
        draw_text(">> Điều khiển:", lx + 20, y, FONT_SIZE_SMALL, COLOR_CYAN);
        y += 22;
        draw_text("   Các phím Mũi tên", lx + 20, y, FONT_SIZE_SMALL, COLOR_DIM_WHITE);
        y += 35;

        draw_text(">> Luật chơi:", lx + 20, y, FONT_SIZE_SMALL, COLOR_CYAN);
        y += 22;
        draw_text("   Đâm tiểu hành tinh -> Câu hỏi", lx + 20, y, FONT_SIZE_SMALL, COLOR_DIM_WHITE);
        y += 22;
        draw_text("   Gõ A/B/C/D để chọn đáp án", lx + 20, y, FONT_SIZE_SMALL, COLOR_DIM_WHITE);
        y += 35;

        draw_text(">> Điểm số:", lx + 20, y, FONT_SIZE_SMALL, COLOR_CYAN);
        y += 22;
        draw_text("   ĐÚNG:  +10 Điểm", lx + 20, y, FONT_SIZE_SMALL, COLOR_NEON_GREEN);
        y += 22;
        draw_text("   SAI / HẾT GIỜ: -1 Máu", lx + 20, y, FONT_SIZE_SMALL, COLOR_RED);
        y += 35;

        draw_text(">> Khiên bảo vệ:", lx + 20, y, FONT_SIZE_SMALL, COLOR_CYAN);
        y += 22;
        draw_text("   Ăn khiên -> Chống địch 10s", lx + 20, y, FONT_SIZE_SMALL, (SDL_Color){50, 150, 255, 255});
        y += 22;
        draw_text("   Nhấp nháy khi sắp hết!", lx + 20, y, FONT_SIZE_SMALL, COLOR_DIM_WHITE);
        y += 35;

        draw_text(">> Lưu ý:", lx + 20, y, FONT_SIZE_SMALL, COLOR_CYAN);
        y += 22;
        draw_text("   TĂNG TỐC sau mỗi 3 giây!", lx + 20, y, FONT_SIZE_SMALL, COLOR_NEON_ORANGE);
        y += 22;
        draw_text("   Tàu địch tăng dần!", lx + 20, y, FONT_SIZE_SMALL, COLOR_RED);

        // === CỘT PHẢI: Bảng ký hiệu ===
        int rx = 445, ry = 70;
        int rbox_w = 325, rbox_h = 440;
        draw_filled_rect(rx, ry, rbox_w, rbox_h, (SDL_Color){15, 15, 40, 220});
        draw_rect_outline(rx, ry, rbox_w, rbox_h, COLOR_CYAN);

        // Title inside the right box
        draw_text("KÝ HIỆU", rx + rbox_w / 2 - 50, ry + 10, FONT_SIZE_MEDIUM, COLOR_CYAN);

        // 1. Tàu của bạn
        int sy = ry + 50;
        draw_ship(rx + 30, sy);
        draw_text("Tàu của bạn", rx + 80, sy + 8, FONT_SIZE_SMALL, COLOR_WHITE);

        // 2. Tiểu hành tinh chứa năng lượng
        sy += 65;
        draw_asteroid(rx + 25, sy, 0.5f);
        draw_text("Tiểu hành tinh", rx + 80, sy + 5, FONT_SIZE_SMALL, COLOR_DIM_WHITE);
        draw_text("(Chứa năng lượng)", rx + 80, sy + 25, FONT_SIZE_SMALL, COLOR_DIM_WHITE);

        // 3. Tàu địch
        sy += 70;
        draw_enemy_ship(rx + 30, sy);
        draw_text("Tàu địch", rx + 80, sy + 5, FONT_SIZE_SMALL, COLOR_RED);
        draw_text("(Mất mạng nếu chạm!)", rx + 80, sy + 25, FONT_SIZE_SMALL, COLOR_RED);

        // 4. Khiên bảo vệ
        sy += 70;
        draw_shield_pickup(rx + 33, sy);
        draw_text("Khiên bảo vệ", rx + 80, sy + 2, FONT_SIZE_SMALL, (SDL_Color){50, 150, 255, 255});
        draw_text("(Chống địch 10s)", rx + 80, sy + 22, FONT_SIZE_SMALL, (SDL_Color){50, 150, 255, 255});

        // 5. Hiệu ứng khiên
        sy += 70;
        draw_ship(rx + 30, sy);
        draw_shield_effect(rx + 30, sy, false);
        draw_text("Đang có khiên", rx + 80, sy + 8, FONT_SIZE_SMALL, (SDL_Color){100, 200, 255, 255});

        // Button
        Uint32 blink = (SDL_GetTicks() / 500) % 2;
        if (blink) {
            draw_text_centered("[ SPACE: Bắt đầu chơi! ]", SCREEN_HEIGHT - 30,
                              FONT_SIZE_SMALL, COLOR_NEON_GREEN);
        }

        present_screen();
        SDL_Delay(FRAME_DELAY);
    }
}
