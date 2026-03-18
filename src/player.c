#include "game.h"
#include "ui.h"
#include "graphics.h"
#include "story.h"
#include "timer.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

int net_score = 0;
int net_correct = 0;
int net_wrong = 0;

#define MAX_ASTEROIDS 5
#define MAX_HISTORY 100
#define SHIP_SPEED 5
#define ASTEROID_BASE_SPEED 2.0f

typedef struct {
    float x, y;
    float rotation;
    float speed;
    bool active;
} Asteroid_SDL;

typedef struct {
    int qIndex;
    char userAnswer;
    char correctAnswer;
} AnswerHistory;

// BIẾN TOÀN CỤC CHO LỊCH SỬ CÂU HỎI
AnswerHistory game_history[MAX_HISTORY];
int game_history_count = 0;

// ==========================================
// MÀN HÌNH XEM LẠI CÂU HỎI
// ==========================================
void showReviewScreen() {
    if (game_history_count == 0) {
        show_sdl_message("Ban chua tra loi cau nao!");
        return;
    }

    int current = 0;
    bool done = false;

    while (!done) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) { done = true; break; }
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_q: case SDLK_ESCAPE: done = true; break;
                    case SDLK_LEFT: case SDLK_a:
                        if (current > 0) current--;
                        break;
                    case SDLK_RIGHT: case SDLK_d:
                        if (current < game_history_count - 1) current++;
                        break;
                    default: break;
                }
            }
        }

        clear_screen();
        update_and_draw_stars();

        // Title
        draw_text_centered("XEM LẠI CÂU HỎI", 30, FONT_SIZE_LARGE, COLOR_CYAN);

        int qIdx = game_history[current].qIndex;
        
        // Câu số
        char num_str[32];
        snprintf(num_str, sizeof(num_str), "Cau %d / %d", current + 1, game_history_count);
        draw_text_centered(num_str, 70, FONT_SIZE_SMALL, COLOR_DIM_WHITE);

        // Box câu hỏi
        int box_x = 50, box_y = 100, box_w = 700;
        
        // Tính chiều cao chữ bằng alpha=0
        int consumed_h = draw_text_wrapped(qBank[qIdx].question, box_x + 20, box_y + 20, box_w - 40, 24, FONT_SIZE_MEDIUM, (SDL_Color){0,0,0,0});
        int box_h = 240 + consumed_h; 
        if (box_h < 350) box_h = 350;

        draw_filled_rect(box_x, box_y, box_w, box_h, (SDL_Color){20, 20, 50, 220});
        draw_rect_outline(box_x, box_y, box_w, box_h, COLOR_CYAN);

        // Câu hỏi
        draw_text_wrapped(qBank[qIdx].question, box_x + 20, box_y + 20, box_w - 40, 24, FONT_SIZE_MEDIUM, COLOR_WHITE);

        // Đáp án
        char opt_a[120], opt_b[120], opt_c[120], opt_d[120];
        snprintf(opt_a, sizeof(opt_a), "A. %s", qBank[qIdx].optionA);
        snprintf(opt_b, sizeof(opt_b), "B. %s", qBank[qIdx].optionB);
        snprintf(opt_c, sizeof(opt_c), "C. %s", qBank[qIdx].optionC);
        snprintf(opt_d, sizeof(opt_d), "D. %s", qBank[qIdx].optionD);

        SDL_Color normal = COLOR_DIM_WHITE;
        SDL_Color correct_color = COLOR_NEON_GREEN;
        SDL_Color wrong_color = COLOR_RED;

        char correct = game_history[current].correctAnswer;
        char user = game_history[current].userAnswer;

        int opt_base_y = box_y + 40 + consumed_h;

        // Highlight đáp án đúng/sai
        draw_text(opt_a, box_x + 40, opt_base_y, FONT_SIZE_SMALL,
            correct == 'A' ? correct_color : (user == 'A' ? wrong_color : normal));
        draw_text(opt_b, box_x + 40, opt_base_y + 35, FONT_SIZE_SMALL,
            correct == 'B' ? correct_color : (user == 'B' ? wrong_color : normal));
        draw_text(opt_c, box_x + 40, opt_base_y + 70, FONT_SIZE_SMALL,
            correct == 'C' ? correct_color : (user == 'C' ? wrong_color : normal));
        draw_text(opt_d, box_x + 40, opt_base_y + 105, FONT_SIZE_SMALL,
            correct == 'D' ? correct_color : (user == 'D' ? wrong_color : normal));

        int res_y = opt_base_y + 160;

        // Kết quả
        char ans_str[64];
        snprintf(ans_str, sizeof(ans_str), "Bạn chọn: %c", user);
        draw_text(ans_str, box_x + 40, res_y, FONT_SIZE_MEDIUM,
            user == correct ? COLOR_NEON_GREEN : COLOR_RED);

        char correct_str[64];
        snprintf(correct_str, sizeof(correct_str), "Đáp án đúng: %c", correct);
        draw_text(correct_str, box_x + 40, res_y + 35, FONT_SIZE_MEDIUM, COLOR_NEON_GREEN);

        // Trạng thái
        if (user == correct) {
            draw_text(">> DUNG!", box_x + 400, res_y, FONT_SIZE_MEDIUM, COLOR_NEON_GREEN);
        } else if (user == '-') {
            draw_text(">> HET GIO!", box_x + 400, res_y, FONT_SIZE_MEDIUM, COLOR_RED);
        } else {
            draw_text(">> SAI!", box_x + 400, res_y, FONT_SIZE_MEDIUM, COLOR_RED);
        }

        // Navigation
        draw_text_centered("[A/Left] Truoc  |  [D/Right] Sau  |  [Q] Thoat",
                          SCREEN_HEIGHT - 40, FONT_SIZE_SMALL, COLOR_DIM_WHITE);

        present_screen();
        SDL_Delay(FRAME_DELAY);
    }
}

// ==========================================
// GAME OVER SCREEN
// ==========================================
void showGameOverScreen() {
    bool done = false;
    float anim = 0;

    while (!done) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) { done = true; break; }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_SPACE) {
                    showReviewScreen();
                    done = true;
                } else if (e.key.keysym.sym == SDLK_q || e.key.keysym.sym == SDLK_ESCAPE) {
                    done = true;
                }
            }
        }

        anim += 0.03f;
        if (anim > 2 * M_PI) anim -= 2 * M_PI;

        clear_screen();
        update_and_draw_stars();

        // Title pulse
        int glow = 200 + (int)(sinf(anim) * 55);
        draw_text_centered("GAME OVER", 100, FONT_SIZE_LARGE, (SDL_Color){255, (Uint8)glow, (Uint8)glow, 255});

        // Score box
        int box_x = 200, box_y = 180, box_w = 400, box_h = 200;
        draw_filled_rect(box_x, box_y, box_w, box_h, (SDL_Color){20, 20, 50, 220});
        draw_rect_outline(box_x, box_y, box_w, box_h, COLOR_CYAN);

        char s_str[50], c_str[50], w_str[50];
        snprintf(s_str, sizeof(s_str), "Tổng điểm: %3d", net_score);
        snprintf(c_str, sizeof(c_str), "Câu đúng : %3d", net_correct);
        snprintf(w_str, sizeof(w_str), "Câu sai  : %3d", net_wrong);

        int text_x = SCREEN_WIDTH / 2 - 95;
        draw_text(s_str, text_x, box_y + 40, FONT_SIZE_MEDIUM, COLOR_NEON_GREEN);
        draw_text(c_str, text_x, box_y + 80, FONT_SIZE_MEDIUM, COLOR_CYAN);
        draw_text(w_str, text_x, box_y + 120, FONT_SIZE_MEDIUM, COLOR_RED);

        // Buttons
        float blink = sinf(anim * 3) > 0 ? 1.0f : 0.6f;
        SDL_Color space_color = {0, (Uint8)(255 * blink), (Uint8)(255 * blink), 255};
        draw_text_centered("[ SPACE: Xem lại câu hỏi ]", 420, FONT_SIZE_SMALL, space_color);
        draw_text_centered("[ Q: Về Lại Menu ]", 460, FONT_SIZE_SMALL, COLOR_DIM_WHITE);

        present_screen();
        SDL_Delay(FRAME_DELAY);
    }
}

// ==========================================
// HỎI CÂU HỎI KHI VA CHẠM
// ==========================================
void askQuestion(int *score, int *lives, int *last_q_index,
                 int *correct_count, int *wrong_count, bool debug_mode) {
    if (qCount == 0) {
        *score += 10;
        return;
    }

    int qIndex = 0;
    if (qCount > 1) {
        do { qIndex = rand() % qCount; } while (qIndex == *last_q_index);
    }
    *last_q_index = qIndex;

    GameTimer q_timer;
    start_timer(&q_timer, 10);
    char answer = '-';
    bool answered = false;
    int selected = 0; // 0=A, 1=B, 2=C, 3=D

    while (!is_time_up(&q_timer) && !answered) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                answer = '-';
                answered = true;
                break;
            }
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_a: answer = 'A'; answered = true; break;
                    case SDLK_b: answer = 'B'; answered = true; break;
                    case SDLK_c: answer = 'C'; answered = true; break;
                    case SDLK_d: answer = 'D'; answered = true; break;
                    case SDLK_UP:
                        selected = (selected - 1 + 4) % 4;
                        break;
                    case SDLK_DOWN:
                        selected = (selected + 1) % 4;
                        break;
                    case SDLK_RETURN:
                        answer = 'A' + selected;
                        answered = true;
                        break;
                    default: break;
                }
            }
        }

        int remaining = get_remaining_time(&q_timer);
        float progress = (float)remaining / 10.0f;

        clear_screen();
        update_and_draw_stars();
        draw_overlay();

        // Title
        draw_text_centered("CÂU HỎI", 40, FONT_SIZE_LARGE, COLOR_YELLOW);

        // Debug mode
        if (debug_mode) {
            char debug_str[64];
            snprintf(debug_str, sizeof(debug_str), "[DEBUG: Đáp án đúng là %c]", qBank[qIndex].correctOption);
            draw_text_centered(debug_str, 75, FONT_SIZE_SMALL, COLOR_RED);
        }

        // Tính chiều cao câu hỏi bằng cách render nháp
        int qbox_x = 60, qbox_y = 100, qbox_w = 680;
        int consumed_h = draw_text_wrapped(qBank[qIndex].question, qbox_x + 20, qbox_y + 20, qbox_w - 40, 24, FONT_SIZE_MEDIUM, (SDL_Color){0,0,0,0});
        int qbox_h = 240 + consumed_h;
        if (qbox_h < 350) qbox_h = 350;

        // Question box
        draw_filled_rect(qbox_x, qbox_y, qbox_w, qbox_h, (SDL_Color){15, 15, 40, 240});
        draw_rect_outline(qbox_x, qbox_y, qbox_w, qbox_h, COLOR_YELLOW);

        // Question text
        draw_text_wrapped(qBank[qIndex].question, qbox_x + 20, qbox_y + 20, qbox_w - 40, 24, FONT_SIZE_MEDIUM, COLOR_WHITE);

        // Options
        char opt_texts[4][120];
        snprintf(opt_texts[0], sizeof(opt_texts[0]), "A. %s", qBank[qIndex].optionA);
        snprintf(opt_texts[1], sizeof(opt_texts[1]), "B. %s", qBank[qIndex].optionB);
        snprintf(opt_texts[2], sizeof(opt_texts[2]), "C. %s", qBank[qIndex].optionC);
        snprintf(opt_texts[3], sizeof(opt_texts[3]), "D. %s", qBank[qIndex].optionD);

        int opt_base_y = qbox_y + 50 + consumed_h;

        for (int i = 0; i < 4; i++) {
            int opt_y = opt_base_y + i * 45;
            SDL_Color bg = (i == selected) ?
                (SDL_Color){40, 40, 100, 200} :
                (SDL_Color){25, 25, 55, 200};
            
            draw_filled_rect(qbox_x + 30, opt_y, qbox_w - 60, 35, bg);
            if (i == selected) {
                draw_rect_outline(qbox_x + 30, opt_y, qbox_w - 60, 35, COLOR_CYAN);
            }
            draw_text(opt_texts[i], qbox_x + 45, opt_y + 10, FONT_SIZE_SMALL,
                     i == selected ? COLOR_CYAN : COLOR_DIM_WHITE);
        }

        // Timer circle
        SDL_Color timer_color = remaining > 5 ? COLOR_NEON_GREEN : COLOR_RED;
        draw_progress_circle(SCREEN_WIDTH - 80, 60, 25, progress, timer_color);

        char time_str[8];
        snprintf(time_str, sizeof(time_str), "%02d", remaining);
        draw_text(time_str, SCREEN_WIDTH - 92, 50, FONT_SIZE_MEDIUM, timer_color);

        // Hint
        draw_text_centered("[A/B/C/D] hoặc [Up/Down + ENTER]",
                          SCREEN_HEIGHT - 30, FONT_SIZE_SMALL, COLOR_DIM_WHITE);

        present_screen();
        SDL_Delay(FRAME_DELAY);
    }

    // Record history
    char real_correct = qBank[qIndex].correctOption;
    if (real_correct >= 'a' && real_correct <= 'z') real_correct -= 32;

    if (game_history_count < MAX_HISTORY) {
        game_history[game_history_count].qIndex = qIndex;
        game_history[game_history_count].userAnswer = answer;
        game_history[game_history_count].correctAnswer = real_correct;
        game_history_count++;
    }

    // Update score/lives ONCE
    if (answer == real_correct) {
        *score += 10;
        (*correct_count)++;
    } else if (answer == '-') {
        *lives -= 1;
        (*wrong_count)++;
    } else {
        *lives -= 1;
        (*wrong_count)++;
    }

    // Show result
    Uint32 result_start = SDL_GetTicks();
    while (SDL_GetTicks() - result_start < 1500) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return;
        }

        clear_screen();
        update_and_draw_stars();
        draw_overlay();

        if (answer == real_correct) {
            draw_text_centered("CHÍNH XÁC!", SCREEN_HEIGHT / 2 - 30, FONT_SIZE_LARGE, COLOR_NEON_GREEN);
            draw_text_centered("+10 Điểm", SCREEN_HEIGHT / 2 + 20, FONT_SIZE_MEDIUM, COLOR_NEON_GREEN);
        } else if (answer == '-') {
            draw_text_centered("HẾT GIỜ!", SCREEN_HEIGHT / 2 - 30, FONT_SIZE_LARGE, COLOR_RED);
            char ans_msg[64];
            snprintf(ans_msg, sizeof(ans_msg), "Đáp án đúng là: %c", real_correct);
            draw_text_centered(ans_msg, SCREEN_HEIGHT / 2 + 20, FONT_SIZE_MEDIUM, COLOR_YELLOW);
            draw_text_centered("-1 Máu", SCREEN_HEIGHT / 2 + 50, FONT_SIZE_SMALL, COLOR_RED);
        } else {
            draw_text_centered("SAI RỒI!", SCREEN_HEIGHT / 2 - 30, FONT_SIZE_LARGE, COLOR_RED);
            char ans_msg[64];
            snprintf(ans_msg, sizeof(ans_msg), "Đáp án đúng là: %c", real_correct);
            draw_text_centered(ans_msg, SCREEN_HEIGHT / 2 + 20, FONT_SIZE_MEDIUM, COLOR_YELLOW);
            draw_text_centered("-1 Máu", SCREEN_HEIGHT / 2 + 50, FONT_SIZE_SMALL, COLOR_RED);
        }

        present_screen();
        SDL_Delay(FRAME_DELAY);
    }
}

// ==========================================
// GAME LOOP CHÍNH
// ==========================================
#define MAX_ENEMIES 8
#define MAX_SHIELD_PICKUPS 3
#define ENEMY_BASE_SPEED 1.2f
#define SHIELD_DURATION 10000  // 10 giây (ms)

typedef struct {
    float x, y;
    float speed;
    float zigzag_offset;
    float zigzag_phase;
    bool active;
} EnemyShip;

typedef struct {
    float x, y;
    float speed;
    bool active;
} ShieldPickup;

void playGame() {
    showInstructions();

    int score = 0;
    int lives = 3;
    int last_question_index = -1;
    bool debug_mode = false;
    int correct_count = 0;
    int wrong_count = 0;
    game_history_count = 0;

    int total_time = 60 + (qCount * 5);
    GameTimer main_timer;
    start_timer(&main_timer, total_time);

    // Ship position
    float ship_x = SCREEN_WIDTH / 2.0f;
    float ship_y = SCREEN_HEIGHT - 80.0f;

    srand(time(NULL));

    // Khởi tạo thiên thạch
    Asteroid_SDL asteroids[MAX_ASTEROIDS];
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        asteroids[i].x = 80 + rand() % (SCREEN_WIDTH - 160);
        asteroids[i].y = -(i * 150) - 50;
        asteroids[i].rotation = (float)(rand() % 360) * M_PI / 180.0f;
        asteroids[i].speed = ASTEROID_BASE_SPEED + (float)(rand() % 10) / 10.0f;
        asteroids[i].active = true;
    }

    // Khởi tạo tàu địch
    EnemyShip enemies[MAX_ENEMIES];
    int active_enemy_count = 2; // Bắt đầu với 2 tàu
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].x = 60 + rand() % (SCREEN_WIDTH - 120);
        enemies[i].y = -(i * 200) - 100 - rand() % 200;
        enemies[i].speed = ENEMY_BASE_SPEED + (float)(rand() % 10) / 10.0f;
        enemies[i].zigzag_offset = 0;
        enemies[i].zigzag_phase = (float)(rand() % 628) / 100.0f;
        enemies[i].active = (i < active_enemy_count);
    }

    // Khởi tạo shield pickups
    ShieldPickup shields[MAX_SHIELD_PICKUPS];
    for (int i = 0; i < MAX_SHIELD_PICKUPS; i++) {
        shields[i].x = 80 + rand() % (SCREEN_WIDTH - 160);
        shields[i].y = -(rand() % 500) - 300 - i * 400;
        shields[i].speed = 1.0f;
        shields[i].active = true;
    }

    // Shield state
    bool shield_active = false;
    Uint32 shield_start_time = 0;

    float speed_mult = 1.0f;
    Uint32 last_speedup = SDL_GetTicks();
    Uint32 last_enemy_increase = SDL_GetTicks();
    bool running = true;
    
    bool is_paused = false;
    Uint32 pause_start_ticks = 0;

    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    while (running) {
        Uint32 frame_start = SDL_GetTicks();

        // Events
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) { running = false; break; }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_q) { running = false; break; }
                if (e.key.keysym.sym == SDLK_ESCAPE) { 
                    is_paused = !is_paused; 
                    if (is_paused) {
                        pause_start_ticks = SDL_GetTicks();
                    } else {
                        Uint32 pause_dur = SDL_GetTicks() - pause_start_ticks;
                        last_speedup += pause_dur;
                        last_enemy_increase += pause_dur;
                        if (shield_active) shield_start_time += pause_dur;
                        main_timer.start_time += (pause_dur / 1000);
                    }
                }
                if (e.key.keysym.sym == SDLK_HASH ||
                    (e.key.keysym.sym == SDLK_3 && 
                     (e.key.keysym.mod & KMOD_SHIFT))) {
                    debug_mode = !debug_mode;
                }
            }
        }
        
        if (is_paused) {
            draw_text_centered("[ PAUSED ]", SCREEN_HEIGHT / 2, FONT_SIZE_LARGE, COLOR_YELLOW);
            draw_text_centered("[ ESC: Tiếp tục  |  Q: Thoát ]", SCREEN_HEIGHT / 2 + 40, FONT_SIZE_SMALL, COLOR_DIM_WHITE);
            present_screen();
            SDL_Delay(FRAME_DELAY);
            continue;
        }

        // Check game over
        if ((lives <= 0 || is_time_up(&main_timer)) && !debug_mode) {
            net_score = score;
            net_correct = correct_count;
            net_wrong = wrong_count;
            return;
        }

        if (debug_mode && lives < 3) lives = 3;

        // Input: di chuyển liên tục
        SDL_PumpEvents();
        if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
            ship_x -= SHIP_SPEED;
            if (ship_x < 25) ship_x = 25;
        }
        if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
            ship_x += SHIP_SPEED;
            if (ship_x > SCREEN_WIDTH - 25) ship_x = SCREEN_WIDTH - 25;
        }
        if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
            ship_y -= SHIP_SPEED;
            if (ship_y < 70) ship_y = 70;
        }
        if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
            ship_y += SHIP_SPEED;
            if (ship_y > SCREEN_HEIGHT - 30) ship_y = SCREEN_HEIGHT - 30;
        }

        // Speed up (nhanh hơn: +0.25 mỗi 3 giây)
        if (SDL_GetTicks() - last_speedup >= 3000) {
            speed_mult += 0.18f;
            last_speedup = SDL_GetTicks();
        }

        // Tăng số tàu địch mỗi 15 giây
        if (SDL_GetTicks() - last_enemy_increase >= 10000) {
            if (active_enemy_count < MAX_ENEMIES) {
                enemies[active_enemy_count].x = 60 + rand() % (SCREEN_WIDTH - 120);
                enemies[active_enemy_count].y = -50 - rand() % 200;
                enemies[active_enemy_count].speed = ENEMY_BASE_SPEED + (float)(rand() % 15) / 10.0f;
                enemies[active_enemy_count].zigzag_phase = (float)(rand() % 628) / 100.0f;
                enemies[active_enemy_count].active = true;
                active_enemy_count++;
            }
            last_enemy_increase = SDL_GetTicks();
        }

        // Update asteroids
        for (int i = 0; i < MAX_ASTEROIDS; i++) {
            asteroids[i].y += asteroids[i].speed * speed_mult;
            asteroids[i].rotation += 0.02f;
            if (asteroids[i].y > SCREEN_HEIGHT + 20) {
                asteroids[i].y = -50 - rand() % 100;
                asteroids[i].x = 80 + rand() % (SCREEN_WIDTH - 160);
                asteroids[i].speed = ASTEROID_BASE_SPEED + (float)(rand() % 15) / 10.0f;
            }
        }

        // Update tàu địch (zigzag)
        for (int i = 0; i < active_enemy_count; i++) {
            if (!enemies[i].active) continue;
            enemies[i].y += enemies[i].speed * speed_mult;
            enemies[i].zigzag_phase += 0.05f;
            enemies[i].zigzag_offset = sinf(enemies[i].zigzag_phase) * 2.0f;
            enemies[i].x += enemies[i].zigzag_offset;
            // Giới hạn trong màn hình
            if (enemies[i].x < 20) enemies[i].x = 20;
            if (enemies[i].x > SCREEN_WIDTH - 50) enemies[i].x = SCREEN_WIDTH - 50;
            if (enemies[i].y > SCREEN_HEIGHT + 30) {
                enemies[i].y = -40 - rand() % 200;
                enemies[i].x = 60 + rand() % (SCREEN_WIDTH - 120);
                enemies[i].speed = ENEMY_BASE_SPEED + (float)(rand() % 15) / 10.0f;
            }
        }

        // Update shield pickups
        for (int i = 0; i < MAX_SHIELD_PICKUPS; i++) {
            if (!shields[i].active) continue;
            shields[i].y += shields[i].speed;
            if (shields[i].y > SCREEN_HEIGHT + 20) {
                shields[i].y = -100 - rand() % 500;
                shields[i].x = 80 + rand() % (SCREEN_WIDTH - 160);
            }
        }

        // Kiểm tra shield hết hạn
        if (shield_active) {
            if (SDL_GetTicks() - shield_start_time >= SHIELD_DURATION) {
                shield_active = false;
            }
        }

        // === COLLISION: Thiên thạch ===
        for (int i = 0; i < MAX_ASTEROIDS; i++) {
            float dx = asteroids[i].x - ship_x;
            float dy = asteroids[i].y - ship_y;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist < 35) {
                spawn_particles((int)asteroids[i].x, (int)asteroids[i].y, COLOR_NEON_ORANGE);
                int ask_start = time(NULL);
                askQuestion(&score, &lives, &last_question_index,
                           &correct_count, &wrong_count, debug_mode);
                main_timer.start_time += (time(NULL) - ask_start);
                for (int j = 0; j < MAX_ASTEROIDS; j++) {
                    asteroids[j].x = 80 + rand() % (SCREEN_WIDTH - 160);
                    asteroids[j].y = -(j * 150) - 50;
                    asteroids[j].speed = ASTEROID_BASE_SPEED + (float)(rand() % 15) / 10.0f;
                }
                break;
            }
        }

        // === COLLISION: Tàu địch ===
        for (int i = 0; i < active_enemy_count; i++) {
            if (!enemies[i].active) continue;
            float dx = enemies[i].x - ship_x;
            float dy = enemies[i].y - ship_y;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist < 30) {
                spawn_particles((int)enemies[i].x, (int)enemies[i].y, COLOR_RED);
                if (shield_active) {
                    // Khiên hấp thụ va chạm
                    enemies[i].y = -50 - rand() % 200;
                    enemies[i].x = 60 + rand() % (SCREEN_WIDTH - 120);
                } else {
                    // Mất mạng
                    lives--;
                    enemies[i].y = -50 - rand() % 200;
                    enemies[i].x = 60 + rand() % (SCREEN_WIDTH - 120);
                }
            }
        }

        // === COLLISION: Ăn shield ===
        for (int i = 0; i < MAX_SHIELD_PICKUPS; i++) {
            if (!shields[i].active) continue;
            float dx = shields[i].x - ship_x;
            float dy = shields[i].y - ship_y;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist < 25) {
                shield_active = true;
                shield_start_time = SDL_GetTicks();
                shields[i].y = -200 - rand() % 500;
                shields[i].x = 80 + rand() % (SCREEN_WIDTH - 160);
            }
        }

        // === RENDER ===
        clear_screen();
        update_and_draw_stars();

        // Asteroids
        for (int i = 0; i < MAX_ASTEROIDS; i++) {
            if (asteroids[i].y > -20 && asteroids[i].y < SCREEN_HEIGHT + 20) {
                draw_asteroid((int)asteroids[i].x, (int)asteroids[i].y, asteroids[i].rotation);
            }
        }

        // Tàu địch
        for (int i = 0; i < active_enemy_count; i++) {
            if (!enemies[i].active) continue;
            if (enemies[i].y > -20 && enemies[i].y < SCREEN_HEIGHT + 20) {
                draw_enemy_ship((int)enemies[i].x, (int)enemies[i].y);
            }
        }

        // Shield pickups
        for (int i = 0; i < MAX_SHIELD_PICKUPS; i++) {
            if (!shields[i].active) continue;
            if (shields[i].y > -20 && shields[i].y < SCREEN_HEIGHT + 20) {
                draw_shield_pickup((int)shields[i].x, (int)shields[i].y);
            }
        }

        // Ship
        draw_ship((int)ship_x, (int)ship_y);

        // Hiệu ứng khiên quanh tàu
        if (shield_active) {
            Uint32 shield_remaining = SHIELD_DURATION - (SDL_GetTicks() - shield_start_time);
            bool blinking = (shield_remaining <= 3000); // Nhấp nháy khi còn <= 3 giây
            draw_shield_effect((int)ship_x, (int)ship_y, blinking);
        }

        // Particles
        update_and_draw_particles();

        // HUD
        draw_hud(score, lives, get_remaining_time(&main_timer));

        // Shield indicator trên HUD
        if (shield_active) {
            Uint32 shield_remaining = SHIELD_DURATION - (SDL_GetTicks() - shield_start_time);
            char shield_str[32];
            snprintf(shield_str, sizeof(shield_str), "Khiên: %ds", (int)(shield_remaining / 1000));
            draw_text(shield_str, SCREEN_WIDTH / 2 - 40, 15, FONT_SIZE_SMALL, 
                     (SDL_Color){50, 150, 255, 255});
        }

        // Debug indicator
        if (debug_mode) {
            draw_text("[DEBUG MODE]", 10, SCREEN_HEIGHT - 25, FONT_SIZE_SMALL, COLOR_RED);
        }

        present_screen();

        // Frame rate limiter
        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frame_time);
        }
    }
}