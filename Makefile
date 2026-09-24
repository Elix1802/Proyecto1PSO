CC = gcc

TARGET = compressor

SRC = main.c algorithms.c dataStructs/*.c MD5/*.c

CFLAGS = -Wall -Wextra `pkg-config --cflags gtk4`
LIBS = `pkg-config --libs gtk4`

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LIBS)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean