#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_QUESTIONS 100
#define FILE_NAME "questions.txt"
// Đưa các thông số màn hình vào Header chung
#define WIDTH 50
#define HEIGHT 20

#define MAX_QUESTIONS 100
#define FILE_NAME "questions.txt"

// Cấu trúc dữ liệu cho một câu hỏi
typedef struct {
    char question[256];
    char optionA[100];
    char optionB[100];
    char optionC[100];
    char optionD[100];
    char correctOption;
} Question;

// Sử dụng extern để báo cho trình biên dịch biết các biến này 
// được định nghĩa ở một file .c khác (cụ thể là data.c)
extern Question qBank[MAX_QUESTIONS];
extern int qCount;

// Khai báo nguyên mẫu hàm (Function Prototypes)
void clearBuffer();
void loadQuestions();
void saveQuestions();
void adminMenu();
void playGame();
void showGameOverScreen();

#endif