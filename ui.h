#ifndef UI_H
#define UI_H

#include "graphics.h"
#include <stdbool.h>

// Menu functions - trả về lựa chọn của người dùng
int show_main_menu(void);
int show_admin_menu(void);

// Input & message wrappers
bool get_input_string(int row, int col, char *out_str, int max_len, bool is_password, bool allow_letters);
void show_message(const char *msg);

#endif