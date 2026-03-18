#include "game.h"
#include "ui.h"
#include "graphics.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>

char admin_password[20] = "admin";

// ==========================================
// HÀM IMPORT CSV
// ==========================================
void importFromCSV(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        show_sdl_message("Lỗi: Không tìm thấy file de_thi.csv!");
        return;
    }

    char line[1024];
    int imported_count = 0;
    int dup_count = 0;

    fgets(line, sizeof(line), file); // Bỏ header

    while (fgets(line, sizeof(line), file) && qCount < MAX_QUESTIONS) {
        line[strcspn(line, "\n")] = 0;
        line[strcspn(line, "\r")] = 0;

        char *q = strtok(line, ",");
        char *a = strtok(NULL, ",");
        char *b = strtok(NULL, ",");
        char *c = strtok(NULL, ",");
        char *d = strtok(NULL, ",");
        char *ans = strtok(NULL, ",");

        if (q && a && b && c && d && ans) {
            // Check for duplicate question
            bool is_duplicate = false;
            for (int i = 0; i < qCount; i++) {
                if (strcmp(qBank[i].question, q) == 0) {
                    is_duplicate = true;
                    break;
                }
            }

            if (is_duplicate) {
                dup_count++;
                continue;
            }

            strcpy(qBank[qCount].question, q);
            strcpy(qBank[qCount].optionA, a);
            strcpy(qBank[qCount].optionB, b);
            strcpy(qBank[qCount].optionC, c);
            strcpy(qBank[qCount].optionD, d);
            qBank[qCount].correctOption = ans[0];
            qCount++;
            imported_count++;
        }
    }

    fclose(file);
    if (imported_count > 0) saveQuestions();

    char msg[200];
    if (dup_count > 0) {
        sprintf(msg, "Đã Import %d câu. Bỏ qua %d câu trùng lặp!", imported_count, dup_count);
    } else {
        sprintf(msg, "Đã Import %d câu hỏi thành công!", imported_count);
    }
    show_sdl_message(msg);
}

// ==========================================
// HÀM THÊM VÀ SỬA CÂU HỎI THEO FORM
// ==========================================
void addOrEditQuestion(int index) {
    if (index == -1 && qCount >= MAX_QUESTIONS) {
        show_sdl_message("Lỗi: Đã đạt số lượng câu hỏi tối đa!");
        return;
    }
    
    char fields[6][256] = {"", "", "", "", "", ""};
    if (index >= 0 && index < qCount) {
        strcpy(fields[0], qBank[index].question);
        strcpy(fields[1], qBank[index].optionA);
        strcpy(fields[2], qBank[index].optionB);
        strcpy(fields[3], qBank[index].optionC);
        strcpy(fields[4], qBank[index].optionD);
        fields[5][0] = qBank[index].correctOption;
        fields[5][1] = '\0';
    }
    
    int current_field = 0;
    const char *labels[6] = { "Câu hỏi", "Đáp án A", "Đáp án B", "Đáp án C", "Đáp án D", "Đáp án đúng" };
    bool done = false;
    bool saved = false;
    char composition[32] = "";
    Uint32 ime_last_active = 0;

    SDL_Event flush_e;
    while(SDL_PollEvent(&flush_e));
    
    SDL_StartTextInput();
    while (!done) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) { done = true; }
            if (e.type == SDL_KEYDOWN) {
                bool ime_active = (strlen(composition) > 0) || (SDL_GetTicks() - ime_last_active < 150);
                
                if (current_field == 5) {
                    int k = e.key.keysym.sym;
                    if (k == SDLK_a || k == SDLK_b || k == SDLK_c || k == SDLK_d) {
                        fields[5][0] = k - 32; // In hoa
                        fields[5][1] = '\0';
                        composition[0] = '\0';
                    }
                }
                
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    done = true; // Hủy
                } else if (e.key.keysym.sym == SDLK_UP) {
                    if (!ime_active && current_field > 0) current_field--;
                    composition[0] = '\0';
                } else if (e.key.keysym.sym == SDLK_DOWN) {
                    if (!ime_active && current_field < 5) current_field++;
                    composition[0] = '\0';
                } else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
                    if (!ime_active) {
                        if (current_field < 5) {
                            current_field++;
                            composition[0] = '\0';
                        } else {
                            saved = true; done = true; // Submit form
                        }
                    }
                } else if (e.key.keysym.sym == SDLK_BACKSPACE && !ime_active) {
                    int len = strlen(fields[current_field]);
                    if (len > 0) {
                        do { fields[current_field][--len] = '\0'; } 
                        while(len > 0 && (fields[current_field][len] & 0xC0) == 0x80);
                    }
                }
            } else if (e.type == SDL_TEXTEDITING) {
                if (current_field != 5) {
                    strcpy(composition, e.edit.text);
                    if (strlen(composition) > 0) ime_last_active = SDL_GetTicks();
                } else {
                    composition[0] = '\0';
                }
            } else if (e.type == SDL_TEXTINPUT) {
                composition[0] = '\0';
                if (current_field != 5 && strlen(fields[current_field]) + strlen(e.text.text) < 250) {
                    strcat(fields[current_field], e.text.text);
                }
            }
        }
        
        clear_screen();
        update_and_draw_stars();
        draw_overlay();
        draw_text_centered(index == -1 ? "THÊM CÂU HỎI MỚI" : "SỬA CÂU HỎI", 40, FONT_SIZE_LARGE, COLOR_CYAN);
        
        for (int i = 0; i < 6; i++) {
            SDL_Color color = (current_field == i) ? COLOR_YELLOW : COLOR_DIM_WHITE;
            draw_text(labels[i], 100, 120 + i * 55, FONT_SIZE_MEDIUM, color);
            char display_str[300];
            if (current_field == i) {
                snprintf(display_str, sizeof(display_str), "%s%s_", fields[i], composition);
            } else {
                snprintf(display_str, sizeof(display_str), "%s", fields[i]);
            }
            
            draw_filled_rect(280, 110 + i * 55, 500, 40, (SDL_Color){20, 20, 40, 255});
            if (current_field == i) draw_rect_outline(280, 110 + i * 55, 500, 40, COLOR_YELLOW);
            
            draw_text(display_str, 290, 120 + i * 55, FONT_SIZE_MEDIUM, COLOR_WHITE);
        }
        
        draw_text_centered("[Lên/Xuống] Chọn ô  |  [Enter] Chuyển tiếp/Lưu  |  [ESC] Hủy Thoát", SCREEN_HEIGHT - 40, FONT_SIZE_SMALL, COLOR_DIM_WHITE);
        present_screen();
        SDL_Delay(FRAME_DELAY);
    }
    SDL_StopTextInput();
    
    if (saved) {
        if (strlen(fields[0])==0 || fields[5][0]=='\0') {
             show_sdl_message("Lỗi: Bỏ trống câu hỏi hoặc đáp án trắc nghiệm!");
             return;
        }
        Question *q;
        if (index == -1) {
            q = &qBank[qCount];
            qCount++;
        } else {
            q = &qBank[index];
        }
        strcpy(q->question, fields[0]);
        strcpy(q->optionA, fields[1]);
        strcpy(q->optionB, fields[2]);
        strcpy(q->optionC, fields[3]);
        strcpy(q->optionD, fields[4]);
        char t = fields[5][0];
        if (t >= 'a' && t <= 'd') t -= 32;
        q->correctOption = t;
        
        saveQuestions();
        show_sdl_message(index == -1 ? "Đã thêm câu hỏi!" : "Đã cập nhật câu hỏi!");
    }
}

// ==========================================
// ADMIN MENU
// ==========================================
void adminMenu() {
    // Nhập mật khẩu
    char input_pass[20];
    bool ok = get_sdl_input_string("NHẬP MẬT KHẨU ADMIN (Q: Thoát):", input_pass, 20, true, true);
    if (!ok) return;

    if (strlen(input_pass) == 0 || strcmp(input_pass, admin_password) != 0) {
        show_sdl_message("Sai mật khẩu! Truy cập bị từ chối.");
        return;
    }

    show_sdl_message("Đăng nhập thành công!");

    while (1) {
        int choice = show_admin_menu();

        if (choice == 1) {
            addOrEditQuestion(-1);
        }
        else if (choice == 2) {
            // Xem danh sách câu hỏi
            if (qCount == 0) {
                show_sdl_message("Chưa có câu hỏi nào.");
                continue;
            }

            int page = 0;
            int per_page = 3;
            int selected_index = 0;
            bool viewing = true;

            while (viewing) {
                SDL_Event e;
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_QUIT) { viewing = false; break; }
                    if (e.type == SDL_KEYDOWN) {
                        if (e.key.keysym.sym == SDLK_q || e.key.keysym.sym == SDLK_ESCAPE) {
                            viewing = false;
                        }
                        if (e.key.keysym.sym == SDLK_RIGHT || e.key.keysym.sym == SDLK_d) {
                            if ((page + 1) * per_page < qCount) { 
                                page++; 
                                selected_index = 0; 
                            }
                        }
                        if (e.key.keysym.sym == SDLK_LEFT || e.key.keysym.sym == SDLK_a) {
                            if (page > 0) { 
                                page--; 
                                selected_index = 0; 
                            }
                        }
                        if (e.key.keysym.sym == SDLK_DOWN) {
                            int max_idx = ((page + 1) * per_page <= qCount ? per_page : qCount - page * per_page) - 1;
                            if (selected_index < max_idx) selected_index++;
                        }
                        if (e.key.keysym.sym == SDLK_UP) {
                            if (selected_index > 0) selected_index--;
                        }
                        if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
                            int actual_id = page * per_page + selected_index;
                            if (actual_id < qCount) {
                                addOrEditQuestion(actual_id);
                            }
                        }
                        if (e.key.keysym.sym == SDLK_BACKSPACE || e.key.keysym.sym == SDLK_DELETE) {
                            int actual_id = page * per_page + selected_index;
                            if (actual_id < qCount) {
                                for(int k = actual_id; k < qCount - 1; k++) {
                                    qBank[k] = qBank[k+1];
                                }
                                qCount--;
                                saveQuestions();
                                show_sdl_message("Đã xóa câu hỏi!");
                                
                                int max_idx = ((page + 1) * per_page <= qCount ? per_page : qCount - page * per_page) - 1;
                                if (selected_index > max_idx && max_idx >= 0) selected_index = max_idx;
                                if (max_idx < 0) {
                                    if (page > 0) {
                                        page--;
                                        selected_index = per_page - 1;
                                    } else {
                                        viewing = false;
                                    }
                                }
                            }
                        }
                    }
                }

                clear_screen();
                update_and_draw_stars();

                draw_text_centered("DANH SÁCH CÂU HỎI", 30, FONT_SIZE_LARGE, COLOR_NEON_ORANGE);

                char page_str[32];
                int total_pages = (qCount + per_page - 1) / per_page;
                snprintf(page_str, sizeof(page_str), "Trang %d / %d", page + 1, total_pages);
                draw_text_centered(page_str, 65, FONT_SIZE_SMALL, COLOR_DIM_WHITE);

                int start = page * per_page;
                int end = start + per_page;
                if (end > qCount) end = qCount;

                int y = 95;
                for (int i = start; i < end; i++) {
                    // Box cho mỗi câu hỏi
                    SDL_Color box_bg = (i - start == selected_index) ? (SDL_Color){40, 40, 80, 200} : (SDL_Color){20, 20, 50, 200};
                    SDL_Color box_border = (i - start == selected_index) ? COLOR_YELLOW : COLOR_MID_GRAY;
                    draw_filled_rect(60, y, 680, 135, box_bg);
                    draw_rect_outline(60, y, 680, 135, box_border);

                    char num_str[32];
                    snprintf(num_str, sizeof(num_str), "Câu %d:", i + 1);
                    draw_text(num_str, 80, y + 10, FONT_SIZE_SMALL, COLOR_CYAN);
                    draw_text(qBank[i].question, 80, y + 30, FONT_SIZE_SMALL, COLOR_WHITE);

                    char opt_a[128], opt_b[128], opt_c[128], opt_d[128];
                    snprintf(opt_a, sizeof(opt_a), "A. %s", qBank[i].optionA);
                    snprintf(opt_b, sizeof(opt_b), "B. %s", qBank[i].optionB);
                    snprintf(opt_c, sizeof(opt_c), "C. %s", qBank[i].optionC);
                    snprintf(opt_d, sizeof(opt_d), "D. %s", qBank[i].optionD);

                    draw_text(opt_a, 90, y + 55, FONT_SIZE_SMALL, COLOR_DIM_WHITE);
                    draw_text(opt_b, 400, y + 55, FONT_SIZE_SMALL, COLOR_DIM_WHITE);
                    draw_text(opt_c, 90, y + 80, FONT_SIZE_SMALL, COLOR_DIM_WHITE);
                    draw_text(opt_d, 400, y + 80, FONT_SIZE_SMALL, COLOR_DIM_WHITE);

                    char ans_str[32];
                    snprintf(ans_str, sizeof(ans_str), "Đáp án đúng: %c", qBank[i].correctOption);
                    draw_text(ans_str, 80, y + 105, FONT_SIZE_SMALL, COLOR_NEON_GREEN);

                    y += 150;
                }

                draw_text_centered("[Lên/Xuống] Chọn bài  |  [Trái/Phải] Chuyển trang",
                                  SCREEN_HEIGHT - 55, FONT_SIZE_SMALL, COLOR_DIM_WHITE);
                draw_text_centered("[Enter] Sửa  |  [Del] Xóa  |  [Q] Thoát",
                                  SCREEN_HEIGHT - 30, FONT_SIZE_SMALL, COLOR_DIM_WHITE);

                present_screen();
                SDL_Delay(FRAME_DELAY);
            }
        }
        else if (choice == 3) {
            // Đổi mật khẩu
            char new_pass[20], confirm_pass[20];
            bool ok1 = get_sdl_input_string("Nhập mật khẩu mới (Q: Thoát):", new_pass, 20, true, true);
            if (!ok1) continue;
            
            bool ok2 = get_sdl_input_string("Xác nhận mật khẩu:", confirm_pass, 20, true, true);
            if (!ok2) continue;

            if (strcmp(new_pass, confirm_pass) == 0 && strlen(new_pass) > 0) {
                strcpy(admin_password, new_pass);
                show_sdl_message("Đổi mật khẩu thành công!");
            } else {
                show_sdl_message("Mật khẩu không khớp hoặc để trống!");
            }
        }
        else if (choice == 4) {
            // Đặt lại bộ câu hỏi
            char confirm[10];
            if (get_sdl_input_string("Nhập 'o' để xác nhận xóa (Q: Thoát):", confirm, 9, false, true)) {
                if (strcmp(confirm, "o") == 0) {
                    qCount = 0;
                    saveQuestions();
                    show_sdl_message("Đã xóa toàn bộ câu hỏi!");
                } else {
                    show_sdl_message("Đã hủy thao tác xóa.");
                }
            }
        }
        else if (choice == 5) {
            // Import CSV
            char confirm[10];
            if (get_sdl_input_string("File 'de_thi.csv'. Nhập 'o' để nạp (Q: Thoát):", confirm, 9, false, true)) {
                if (strcmp(confirm, "o") == 0) {
                    importFromCSV("de_thi.csv");
                } else {
                    show_sdl_message("Đã hủy nạp dữ liệu.");
                }
            }
        }
        else if (choice == 6) {
            // Nhóm tính năng: Tạo file CSV mẫu
            FILE *f = fopen("de_thi_mau.csv", "w");
            if (f) {
                // Ghi bộ font UTF8 nếu cần thiết, hoặc ghi text thường, MS Excel cần BOM, nhưng ta chỉ ghi CSV chuẩn
                // Thêm BOM (Byte Order Mark) để Excel mở tiếng Việt không bị lỗi font:
                fprintf(f, "\xEF\xBB\xBF");
                fprintf(f, "Cau_Hoi,Dap_An_A,Dap_An_B,Dap_An_C,Dap_An_D,Dap_An_Dung\n");
                fprintf(f, "Thủ đô của Việt Nam là?,Hồ Chí Minh,Hà Nội,Đà Nẵng,Hải Phòng,B\n");
                fprintf(f, "Hành tinh lớn nhất hệ Mặt Trời?,Trái Đất,Sao Hỏa,Sao Mộc,Sao Kim,C\n");
                fclose(f);
                show_sdl_message("Đã tạo file mẫu 'de_thi_mau.csv' tại thư mục game!");
            } else {
                show_sdl_message("Lỗi: Không thể tạo file mẫu!");
            }
        }
        else if (choice == 0) {
            break;
        }
    }
}