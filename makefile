CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -Iinclude

SRC = src/main.c src/storage.c
TARGET = mfa

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	del mfa.exe 2>nul || true
