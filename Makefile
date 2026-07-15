CC = gcc
CPPFLAGS = -I./include/
CFLAGS = -pedantic -Wall -Wextra -Wvla -Werror
LDFLAGS = -lm -lSDL2

SRC_DIR = src

SRC = $(SRC_DIR)/main.c $(wildcard $(SRC_DIR)/*/*.c)
OBJ = $(SRC:.c=.o) 
TARGET = 3DR

.PHONY: cube clean all

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $^ -o $@

doc:
	doxygen Doxyfile

debug: CFLAGS=-DDEBUG -g -fsanitize=address
debug: LDFLAGS+=-g -fsanitize=address
debug: all 

clean:
	$(RM) $(TARGET) $(OBJ)
