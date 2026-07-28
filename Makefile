# Cross-platform Makefile (Supports Linux, macOS/Darwin, and Windows with g++/clang++)
CXX ?= g++
CXXFLAGS ?= -std=c++20 -Wall -Wextra -O2
TARGET := huff
PREFIX ?= $(HOME)/.local
BINDIR ?= $(PREFIX)/bin

all: build

build: $(TARGET)

$(TARGET): main.cpp includes/*.hpp src/*.cpp
	$(CXX) $(CXXFLAGS) main.cpp src/*.cpp -o $(TARGET)

install: $(TARGET)
	install -d "$(DESTDIR)$(BINDIR)"
	install -m 755 "$(TARGET)" "$(DESTDIR)$(BINDIR)/$(TARGET)"

uninstall:
	rm -f "$(DESTDIR)$(BINDIR)/$(TARGET)"

clean:
	rm -f $(TARGET)

.PHONY: all build install uninstall clean
