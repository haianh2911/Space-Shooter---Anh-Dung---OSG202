#include "game.h"

// Định nghĩa biến thực sự ở đây
Question qBank[MAX_QUESTIONS];
int qCount = 0;

void loadQuestions() {
    FILE *file = fopen(FILE_NAME, "r");
    if (file == NULL) {
        file = fopen(FILE_NAME, "w");
        fclose(file);
        return;
    }

    qCount = 0;
    while (fscanf(file, " %[^\n]", qBank[qCount].question) != EOF) {
        fscanf(file, " %[^\n]", qBank[qCount].optionA);
        fscanf(file, " %[^\n]", qBank[qCount].optionB);
        fscanf(file, " %[^\n]", qBank[qCount].optionC);
        fscanf(file, " %[^\n]", qBank[qCount].optionD);
        fscanf(file, " %c", &qBank[qCount].correctOption);
        qCount++;
        if (qCount >= MAX_QUESTIONS) break;
    }
    fclose(file);
}

void saveQuestions() {
    FILE *file = fopen(FILE_NAME, "w");
    if (file == NULL) {
        printf("Loi: Khong the mo file de luu!\n");
        return;
    }
    for (int i = 0; i < qCount; i++) {
        fprintf(file, "%s\n%s\n%s\n%s\n%s\n%c\n", 
                qBank[i].question, qBank[i].optionA, qBank[i].optionB, 
                qBank[i].optionC, qBank[i].optionD, qBank[i].correctOption);
    }
    fclose(file);
}