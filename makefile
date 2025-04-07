# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Source directories
COMMON = common
CLI = cli
GUI = gui

# Source files
COMMON_SRC = $(COMMON)/protocol.c $(COMMON)/transport.c
CLI_SRC = $(CLI)/main_cli.c
GUI_SRC = $(GUI)/main_gui.c

# Output binaries
CLI_BIN = chat_cli
GUI_BIN = chat_gui

# pkg-config GTK flags
GTK_FLAGS = `pkg-config --cflags --libs gtk+-3.0`

# Targets
all: $(CLI_BIN) $(GUI_BIN)

$(CLI_BIN): $(CLI_SRC) $(COMMON_SRC)
	$(CC) $(CFLAGS) -o $@ $(CLI_SRC) $(COMMON_SRC)

$(GUI_BIN): $(GUI_SRC) $(COMMON_SRC)
	$(CC) $(CFLAGS) -o $@ $(GUI_SRC) $(COMMON_SRC) $(GTK_FLAGS) -lpthread

clean:
	rm -f $(CLI_BIN) $(GUI_BIN)

.PHONY: all clean
