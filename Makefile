# Makefile for snowtrix
# A window decoration inspired by cmatrix

# Output binary name
TARGET = snowtrix

# Compiler and flags
CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wextra -Wno-unused-function -std=c99
LDFLAGS ?= -lncursesw -lm

# Installation paths
PREFIX  ?= /usr/local
BINDIR  ?= $(PREFIX)/bin

SRCS = snowtrix.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

install: $(TARGET)
	@echo "Installing $(TARGET) to $(DESTDIR)$(BINDIR)..."
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/

uninstall:
	@echo "Removing $(TARGET) from $(DESTDIR)$(BINDIR)..."
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
