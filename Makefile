# ============================================================================
# Makefile - Space Combat 3D (Computação Gráfica)
# ============================================================================

CXX      = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17

SRC_DIR   = src
BUILD_DIR = build
TARGET    = space_combat

SRCS = $(SRC_DIR)/main.cpp \
       $(SRC_DIR)/core/entidades.cpp \
       $(SRC_DIR)/input/input.cpp \
       $(SRC_DIR)/fisica/logica.cpp \
       $(SRC_DIR)/render/modelos_3d.cpp \
       $(SRC_DIR)/render/cena_3d.cpp \
       $(SRC_DIR)/render/hud.cpp

OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS = $(SRC_DIR)/core/entidades.h \
       $(SRC_DIR)/input/input.h \
       $(SRC_DIR)/fisica/logica.h \
       $(SRC_DIR)/render/modelos_3d.h \
       $(SRC_DIR)/render/cena_3d.h \
       $(SRC_DIR)/render/hud.h

# Detecção automática do Raylib (pkg-config do sistema ou pasta vendor/local)
RAYLIB_PKG_L = $(shell pkg-config --libs raylib 2>/dev/null)
RAYLIB_PKG_C = $(shell pkg-config --cflags raylib 2>/dev/null)

ifneq ($(wildcard vendor/raylib/lib/libraylib.a),)
    RAYLIB_INC = -Ivendor/raylib/include
    LIBS       = -Lvendor/raylib/lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
else ifeq ($(strip $(RAYLIB_PKG_L)),)
    RAYLIB_INC =
    LIBS       = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
else
    RAYLIB_INC = $(RAYLIB_PKG_C)
    LIBS       = $(RAYLIB_PKG_L) -lm
endif

TARGET_CXXFLAGS = $(CXXFLAGS) $(RAYLIB_INC) -I$(SRC_DIR)

# Regra padrão
all: $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS)
	@mkdir -p $(dir $@)
	$(CXX) $(TARGET_CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CXX) $(TARGET_CXXFLAGS) -o $@ $^ $(LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all run clean
