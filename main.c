#include "game.h"
#include "ui.h"
#include "graphics.h"
#include "story.h"
#include <stdlib.h>
#include <time.h>

// Khai báo để main.c biết sự tồn tại của 2 hàm mạng
extern void hostRoom();
extern void joinRoom();

int main(int argc, char *argv[]) {
    // Khởi tạo đồ họa SDL2
    if (init_graphics() < 0) {
        fprintf(stderr, "Khong the khoi tao do hoa!\n");
        return 1;
    }

    loadQuestions(); // Nạp dữ liệu câu hỏi từ file
    
    showIntroSequence(); // Hiệu ứng logo và cốt truyện trước menu

    while (1) {
        int choice = show_main_menu();

        if (choice == 1) {
            playGame();
            showGameOverScreen();
        }
        else if (choice == 2) {
            hostRoom();
        }
        else if (choice == 3) {
            joinRoom();
        }
        else if (choice == 4) {
            adminMenu();
        }
        else if (choice == 0) {
            break; // Thoát game
        }
    }

    // Đóng đồ họa an toàn trước khi thoát
    close_graphics();
    return 0;
}