CC ?= cc
CFLAGS += -Wall -Wextra -O2 $(shell pkg-config --cflags gtk4)
LDLIBS += $(shell pkg-config --libs gtk4) -lm
TARGET = kramerlab-3x3
SRC = src/main.c

.PHONY: all run clean
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDLIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
