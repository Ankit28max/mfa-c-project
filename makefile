CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -Iinclude

SRC = src/main.c
OBJ = $(SRC:.c=.o)
TARGET = mfa

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET) $(OBJ)
