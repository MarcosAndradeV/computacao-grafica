CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17

SRC_DIR   = src
TARGET_3D = main_3d
TARGET_2D = main_raylib

SRCS_3D   = $(SRC_DIR)/main_3d.cpp
SRCS_2D   = $(SRC_DIR)/main_raylib.cpp

RAYLIB_PKG_L = $(shell pkg-config --libs raylib 2>/dev/null)
RAYLIB_PKG_C = $(shell pkg-config --cflags raylib 2>/dev/null)

ifeq ($(strip $(RAYLIB_PKG_L)),)
    LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
else
    LIBS = $(RAYLIB_PKG_L) -lm
endif
TARGET_CXXFLAGS = $(CXXFLAGS) $(RAYLIB_PKG_C)

all: $(TARGET_3D) $(TARGET_2D)

$(TARGET_3D): $(SRCS_3D)
	$(CXX) $(TARGET_CXXFLAGS) -o $@ $^ $(LIBS)

$(TARGET_2D): $(SRCS_2D)
	$(CXX) $(TARGET_CXXFLAGS) -o $@ $^ $(LIBS)

# Aliases
3d: $(TARGET_3D)
2d: $(TARGET_2D)
main: $(TARGET_3D)
raylib: $(TARGET_2D)

run: $(TARGET_3D)
	./$(TARGET_3D)

run-3d: $(TARGET_3D)
	./$(TARGET_3D)

run-2d: $(TARGET_2D)
	./$(TARGET_2D)

clean:
	rm -f $(TARGET_3D) $(TARGET_2D) main

.PHONY: all run run-3d run-2d clean 3d 2d main raylib
