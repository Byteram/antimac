CC = cc
CFLAGS = -O2 -Wall
TARGET = antimac
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

all: $(TARGET)

$(TARGET): antimac.c
	$(CC) $(CFLAGS) -o $(TARGET) antimac.c

install: $(TARGET)
	install -m 0755 $(TARGET) $(BINDIR)/$(TARGET)

remove:
	rm -f $(BINDIR)/$(TARGET)

.PHONY: all install remove 