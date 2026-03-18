# Ten file chay cua game
TARGET = space_shooter

# Trinh bien dich va cac co (flags)
CC = cc

# SDL2 va FreeType2 (da cai qua brew)
SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LIBS = $(shell sdl2-config --libs)

FT_CFLAGS = $(shell pkg-config --cflags freetype2)
FT_LIBS = $(shell pkg-config --libs freetype2)

CFLAGS = -Wall $(SDL_CFLAGS) $(FT_CFLAGS) -Iinclude
LIBS = $(SDL_LIBS) $(FT_LIBS) -lm

# Danh sach toan bo cac file nguon
SRCS = src/main.c src/utils.c src/data.c src/admin.c src/player.c src/story.c src/ui.c src/timer.c src/network.c src/graphics.c

# Mac dinh khi chi go 'make'
all: $(TARGET)

# Lenh bien dich chinh
$(TARGET): $(SRCS)
	TMPDIR=/tmp $(CC) $(SRCS) -o $(TARGET) $(LIBS) $(CFLAGS)
	@echo "=> Bien dich thanh cong! Go 'make run' de choi ngay."

# Lenh don dep file cu
clean:
	rm -f $(TARGET)
	@echo "=> Da xoa file chay cu."

# Lenh bien dich xong va tu dong chay game luon
run: $(TARGET)
	./$(TARGET)