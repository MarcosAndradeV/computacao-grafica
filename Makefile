CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17

SRC_DIR = src

# -------------------------------------------------------------
# 1. Configuração de Flags para GLUT
# -------------------------------------------------------------
GLUT_TARGET = main_glut
GLUT_SRCS   = $(SRC_DIR)/main_glut.cpp
GLUT_PKG_L  = $(shell pkg-config --libs glut glu gl 2>/dev/null)
GLUT_PKG_C  = $(shell pkg-config --cflags glut glu gl 2>/dev/null)

ifeq ($(strip $(GLUT_PKG_L)),)
    GLUT_LIBS = -lGL -lGLU -lglut -lm
else
    GLUT_LIBS = $(GLUT_PKG_L) -lm
endif
GLUT_CXXFLAGS = $(CXXFLAGS) $(GLUT_PKG_C)

# -------------------------------------------------------------
# 2. Configuração de Flags para Raylib
# -------------------------------------------------------------
RAYLIB_TARGET = main_raylib
RAYLIB_SRCS   = $(SRC_DIR)/main_raylib.cpp
RAYLIB_PKG_L  = $(shell pkg-config --libs raylib 2>/dev/null)
RAYLIB_PKG_C  = $(shell pkg-config --cflags raylib 2>/dev/null)

ifeq ($(strip $(RAYLIB_PKG_L)),)
    RAYLIB_LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
else
    RAYLIB_LIBS = $(RAYLIB_PKG_L) -lm
endif
RAYLIB_CXXFLAGS = $(CXXFLAGS) $(RAYLIB_PKG_C)

# -------------------------------------------------------------
# 3. Configuração de Flags para SDL2
# -------------------------------------------------------------
SDL_TARGET = main_sdl
SDL_SRCS   = $(SRC_DIR)/main_sdl.cpp
SDL_PKG_L  = $(shell pkg-config --libs sdl2 gl 2>/dev/null)
SDL_PKG_C  = $(shell pkg-config --cflags sdl2 gl 2>/dev/null)

ifeq ($(strip $(SDL_PKG_L)),)
    SDL_LIBS = -lSDL2 -lGL -lm
else
    SDL_LIBS = $(SDL_PKG_L) -lm
endif
SDL_CXXFLAGS = $(CXXFLAGS) $(SDL_PKG_C)

# -------------------------------------------------------------
# Regras de Construção
# -------------------------------------------------------------
all: $(GLUT_TARGET) $(RAYLIB_TARGET) $(SDL_TARGET)

# Alvos individuais
glut: $(GLUT_TARGET)
raylib: $(RAYLIB_TARGET)
sdl: $(SDL_TARGET)

$(GLUT_TARGET): $(GLUT_SRCS)
	$(CXX) $(GLUT_CXXFLAGS) -o $@ $^ $(GLUT_LIBS)

$(RAYLIB_TARGET): $(RAYLIB_SRCS)
	$(CXX) $(RAYLIB_CXXFLAGS) -o $@ $^ $(RAYLIB_LIBS)

$(SDL_TARGET): $(SDL_SRCS)
	$(CXX) $(SDL_CXXFLAGS) -o $@ $^ $(SDL_LIBS)

# Regras de Execução
run-glut: $(GLUT_TARGET)
	./$(GLUT_TARGET)

run-raylib: $(RAYLIB_TARGET)
	./$(RAYLIB_TARGET)

run-sdl: $(SDL_TARGET)
	./$(SDL_TARGET)

clean:
	rm -f $(GLUT_TARGET) $(RAYLIB_TARGET) $(SDL_TARGET)

.PHONY: all clean glut raylib sdl run-glut run-raylib run-sdl
