CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17

SRC_DIR = src
TARGET  = main_raylib
SRCS    = $(SRC_DIR)/main_raylib.cpp

RAYLIB_PKG_L = $(shell pkg-config --libs raylib 2>/dev/null)
RAYLIB_PKG_C = $(shell pkg-config --cflags raylib 2>/dev/null)

ifeq ($(strip $(RAYLIB_PKG_L)),)
    LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
else
    LIBS = $(RAYLIB_PKG_L) -lm
endif
TARGET_CXXFLAGS = $(CXXFLAGS) $(RAYLIB_PKG_C)

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(TARGET_CXXFLAGS) -o $@ $^ $(LIBS)

main: $(TARGET)
raylib: $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) main

.PHONY: all run clean main raylib
