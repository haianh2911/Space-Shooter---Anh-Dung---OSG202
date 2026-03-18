#ifndef TIMER_H
#define TIMER_H

#include <time.h>
#include <stdbool.h>

// Cấu trúc lưu trữ dữ liệu thời gian
typedef struct {
    time_t start_time;
    int duration; // Tổng thời gian đếm ngược (giây)
} GameTimer;

// Khởi động bộ đếm
void start_timer(GameTimer *t, int seconds);

// Lấy thời gian còn lại
int get_remaining_time(GameTimer *t);

// Kiểm tra xem đã hết giờ chưa
bool is_time_up(GameTimer *t);

#endif