#include "game.h"
#include "ui.h"
#include "graphics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <SDL.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define close closesocket
typedef int socklen_t;
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#endif

#define PORT 8888
#define MAX_PLAYERS 10

extern int net_score;
extern int net_correct;
extern int net_wrong;
void playGame();

typedef struct {
    char name[50];
    int score;
    int correct;
    int wrong;
} PlayerResult;

// Ham lay IP
void get_local_ip(char *buffer) {
    strcpy(buffer, "127.0.0.1");
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) return;
#endif

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return;
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr("8.8.8.8");
    serv.sin_port = htons(53);
    if (connect(sock, (const struct sockaddr*) &serv, sizeof(serv)) == 0) {
        struct sockaddr_in name;
        socklen_t namelen = sizeof(name);
        if (getsockname(sock, (struct sockaddr*) &name, &namelen) == 0) {
            inet_ntop(AF_INET, &name.sin_addr, buffer, 100);
        }
    }
    close(sock);
}

// ==========================================
// MÁY CHỦ: TẠO PHÒNG
// ==========================================
void hostRoom() {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        show_sdl_message("Loi khoi tao mang tren Windows!");
        return;
    }
#endif

    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    int client_sockets[MAX_PLAYERS];
    char player_names[MAX_PLAYERS][50];
    int player_count = 0;

    char local_ip[100];
    get_local_ip(local_ip);

    if (qCount == 0) {
        show_sdl_message("Lỗi: Ngân hàng câu hỏi đang trống!");
        return;
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) return;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(server_fd, FIONBIO, &mode);
#else
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
#endif

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        show_sdl_message("Lỗi: Cổng mạng đang bị dùng!");
        return;
    }
    listen(server_fd, MAX_PLAYERS);

    // SẢNH CHỜ
    bool in_lobby = true;
    while (in_lobby) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                for (int i = 0; i < player_count; i++) close(client_sockets[i]);
                close(server_fd);
                return;
            }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_q) {
                    for (int i = 0; i < player_count; i++) close(client_sockets[i]);
                    close(server_fd);
                    return;
                }
                if (e.key.keysym.sym == SDLK_SPACE && player_count > 0) {
                    in_lobby = false;
                }
            }
        }

        // Accept mới
        int new_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_sock >= 0) {
            if (player_count < MAX_PLAYERS) {
                char temp_name[50] = "Unknown";
                recv(new_sock, temp_name, 50, 0);
                strcpy(player_names[player_count], temp_name);
                client_sockets[player_count] = new_sock;
                player_count++;

                send(new_sock, (const char*)&qCount, sizeof(int), 0);
                send(new_sock, (const char*)qBank, sizeof(Question) * qCount, 0);
            } else {
                close(new_sock);
            }
        }

        // Render lobby
        clear_screen();
        update_and_draw_stars();

        draw_text_centered("SẢNH CHỜ (MÁY CHỦ)", 40, FONT_SIZE_LARGE, COLOR_CYAN);

        char ip_msg[100];
        sprintf(ip_msg, "IP phòng: %s", local_ip);
        draw_text_centered(ip_msg, 90, FONT_SIZE_MEDIUM, COLOR_NEON_GREEN);

        char count_msg[100];
        sprintf(count_msg, "Người chơi: %d / %d", player_count, MAX_PLAYERS);
        draw_text_centered(count_msg, 130, FONT_SIZE_SMALL, COLOR_DIM_WHITE);

        // Danh sách người chơi
        int box_x = 200, box_y = 170, box_w = 400;
        for (int i = 0; i < player_count; i++) {
            draw_filled_rect(box_x, box_y + i * 35, box_w, 30, (SDL_Color){30, 30, 60, 200});
            char name_str[64];
            snprintf(name_str, sizeof(name_str), "%d. %s", i + 1, player_names[i]);
            draw_text(name_str, box_x + 20, box_y + i * 35 + 7, FONT_SIZE_SMALL, COLOR_WHITE);
        }

        if (player_count > 0) {
            Uint32 blink = (SDL_GetTicks() / 500) % 2;
            if (blink) {
                draw_text_centered("[ SPACE: BẮT ĐẦU GAME ]", SCREEN_HEIGHT - 80,
                                  FONT_SIZE_SMALL, COLOR_NEON_GREEN);
            }
        } else {
            draw_text_centered("Đang chờ người chơi vào phòng...", SCREEN_HEIGHT - 80,
                              FONT_SIZE_SMALL, COLOR_DIM_WHITE);
        }
        draw_text_centered("[ Q: Hủy phòng ]", SCREEN_HEIGHT - 40,
                          FONT_SIZE_SMALL, COLOR_DIM_WHITE);

        present_screen();
        SDL_Delay(FRAME_DELAY);
    }

    // PHÁT LỆNH BẮT ĐẦU
    int start_signal = 1;
    for (int i = 0; i < player_count; i++) {
        send(client_sockets[i], (const char*)&start_signal, sizeof(int), 0);
#ifdef _WIN32
        u_long md = 1;
        ioctlsocket(client_sockets[i], FIONBIO, &md);
#else
        int f = fcntl(client_sockets[i], F_GETFL, 0);
        fcntl(client_sockets[i], F_SETFL, f | O_NONBLOCK);
#endif
    }

    // TRACKING LIVE
    int finished_count = 0;
    bool finished[MAX_PLAYERS] = {false};
    PlayerResult results[MAX_PLAYERS];

    while (finished_count < player_count) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) goto cleanup_host;
            if (e.type == SDL_KEYDOWN && (e.key.keysym.sym == SDLK_q)) {
                goto cleanup_host;
            }
        }

        for (int i = 0; i < player_count; i++) {
            if (!finished[i]) {
                int bytes = recv(client_sockets[i], (char*)&results[i], sizeof(PlayerResult), 0);
                if (bytes > 0) {
                    finished[i] = true;
                    finished_count++;
                    close(client_sockets[i]);
                }
            }
        }

        clear_screen();
        update_and_draw_stars();

        draw_text_centered("ĐANG THI ĐẤU...", 40, FONT_SIZE_LARGE, COLOR_YELLOW);

        char wait_msg[100];
        sprintf(wait_msg, "Đã hoàn thành: %d / %d", finished_count, player_count);
        draw_text_centered(wait_msg, 90, FONT_SIZE_SMALL, COLOR_DIM_WHITE);

        for (int i = 0; i < player_count; i++) {
            int y = 130 + i * 35;
            if (finished[i]) {
                draw_filled_rect(200, y, 400, 30, (SDL_Color){20, 60, 20, 200});
                char str[64];
                snprintf(str, sizeof(str), "[x] %-15s : DA XONG", player_names[i]);
                draw_text(str, 220, y + 7, FONT_SIZE_SMALL, COLOR_NEON_GREEN);
            } else {
                draw_filled_rect(200, y, 400, 30, (SDL_Color){30, 30, 60, 200});
                char str[64];
                snprintf(str, sizeof(str), "[ ] %-15s : Dang choi...", player_names[i]);
                draw_text(str, 220, y + 7, FONT_SIZE_SMALL, COLOR_DIM_WHITE);
            }
        }

        draw_text_centered("[ Q: Chốt sớm ]", SCREEN_HEIGHT - 40,
                          FONT_SIZE_SMALL, COLOR_DIM_WHITE);

        present_screen();
        SDL_Delay(100);
    }

cleanup_host:
    close(server_fd);

    // SẮP XẾP VÀ IN BẢNG XẾP HẠNG
    for (int i = 0; i < finished_count - 1; i++) {
        for (int j = i + 1; j < finished_count; j++) {
            if (results[j].score > results[i].score ||
               (results[j].score == results[i].score && results[j].correct > results[i].correct)) {
                PlayerResult temp = results[i];
                results[i] = results[j];
                results[j] = temp;
            }
        }
    }

    bool viewing = true;
    while (viewing) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT || e.type == SDL_KEYDOWN) viewing = false;
        }

        clear_screen();
        update_and_draw_stars();

        draw_text_centered("BẢNG XẾP HẠNG TỔNG KẾT", 40, FONT_SIZE_LARGE, COLOR_YELLOW);

        // Header
        draw_filled_rect(60, 90, 680, 30, (SDL_Color){40, 40, 80, 200});
        draw_text("TOP", 80, 97, FONT_SIZE_SMALL, COLOR_CYAN);
        draw_text("TÊN NGƯỜI CHƠI", 160, 97, FONT_SIZE_SMALL, COLOR_CYAN);
        draw_text("ĐIỂM", 430, 97, FONT_SIZE_SMALL, COLOR_CYAN);
        draw_text("ĐÚNG", 530, 97, FONT_SIZE_SMALL, COLOR_CYAN);
        draw_text("SAI", 630, 97, FONT_SIZE_SMALL, COLOR_CYAN);

        for (int i = 0; i < finished_count; i++) {
            int y = 130 + i * 35;
            SDL_Color row_bg = (i == 0) ?
                (SDL_Color){60, 50, 10, 200} :  // Vàng cho top 1
                (SDL_Color){20, 20, 50, 200};
            draw_filled_rect(60, y, 680, 30, row_bg);

            char rank[8], sc[16], cr[8], wr[8];
            snprintf(rank, sizeof(rank), "#%d", i + 1);
            snprintf(sc, sizeof(sc), "%d", results[i].score);
            snprintf(cr, sizeof(cr), "%d", results[i].correct);
            snprintf(wr, sizeof(wr), "%d", results[i].wrong);

            SDL_Color name_color = (i == 0) ? COLOR_YELLOW : COLOR_WHITE;
            draw_text(rank, 80, y + 7, FONT_SIZE_SMALL, name_color);
            draw_text(results[i].name, 160, y + 7, FONT_SIZE_SMALL, name_color);
            draw_text(sc, 430, y + 7, FONT_SIZE_SMALL, COLOR_NEON_GREEN);
            draw_text(cr, 530, y + 7, FONT_SIZE_SMALL, COLOR_CYAN);
            draw_text(wr, 630, y + 7, FONT_SIZE_SMALL, COLOR_RED);
        }

        draw_text_centered("[ Nhấn phím bất kỳ ]", SCREEN_HEIGHT - 40,
                          FONT_SIZE_SMALL, COLOR_DIM_WHITE);

        present_screen();
        SDL_Delay(FRAME_DELAY);
    }
}

// ==========================================
// MÁY KHÁCH: VÀO PHÒNG
// ==========================================
void joinRoom() {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        show_sdl_message("Loi khoi tao mang tren Windows!");
        return;
    }
#endif

    int sock = 0;
    struct sockaddr_in serv_addr;
    char ip_address[20];
    char player_name[50];

    if (!get_sdl_input_string("Nhập IP phòng máy chủ (Q: Thoát):", ip_address, 19, false, false)) return;
    if (strlen(ip_address) == 0) return;

    if (!get_sdl_input_string("Nhập tên của bạn (Q: Thoát):", player_name, 49, false, true)) return;
    if (strlen(player_name) == 0) return;

    // Show connecting message
    show_sdl_message("Đang kết nối...");

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) return;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, ip_address, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        show_sdl_message("Lỗi: Không thể kết nối vào phòng!");
        return;
    }

    send(sock, player_name, 50, 0);

    int incoming_count;
    recv(sock, (char*)&incoming_count, sizeof(int), 0);
    Question incoming_bank[MAX_QUESTIONS];
    recv(sock, (char*)incoming_bank, sizeof(Question) * incoming_count, 0);

    qCount = incoming_count;
    memcpy(qBank, incoming_bank, sizeof(Question) * incoming_count);

    // Chờ máy chủ bấm bắt đầu
    show_sdl_message("Đã vào phòng! Chờ chủ phòng bắt đầu...");

    int start_signal = 0;
    recv(sock, (char*)&start_signal, sizeof(int), 0);

    playGame();

    // Gửi điểm
    PlayerResult res;
    strncpy(res.name, player_name, 49);
    res.score = net_score;
    res.correct = net_correct;
    res.wrong = net_wrong;
    send(sock, (const char*)&res, sizeof(PlayerResult), 0);

    showGameOverScreen();
    close(sock);
}
