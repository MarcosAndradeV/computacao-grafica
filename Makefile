# ============================================================================
# Makefile - Galeria de Arte 3D Interativa
# Componente: Computação Gráfica
# Tecnologias: C++17, Raylib (OpenGL Moderno com Shaders GLSL)
# ============================================================================

CXX      = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17 -Isrc
LIBS     = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SRC_DIR   = src
BUILD_DIR = build
TARGET    = galeria_arte

SRCS = $(SRC_DIR)/main.cpp \
       $(SRC_DIR)/camera.cpp \
       $(SRC_DIR)/lighting.cpp \
       $(SRC_DIR)/object.cpp \
       $(SRC_DIR)/scene.cpp \
       $(SRC_DIR)/hud.cpp

OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS = $(SRC_DIR)/material.h \
       $(SRC_DIR)/camera.h \
       $(SRC_DIR)/lighting.h \
       $(SRC_DIR)/object.h \
       $(SRC_DIR)/scene.h \
       $(SRC_DIR)/hud.h

all: $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all run clean
