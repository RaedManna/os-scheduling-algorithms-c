CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = scheduler

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) main.c -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) scheduler.exe
