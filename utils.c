#include "game.h"

void clearBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}