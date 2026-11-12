# ============================================================================
# Makefile - Galeria de Arte 3D Interativa
# Componente: Computação Gráfica
# Tecnologias: C++17, OpenGL, GLU, FreeGLUT
# ============================================================================

CXX      = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17
LIBS     = -lglut -lGLU -lGL -lm

TARGET     = galeria_arte
SINGLE_SRC = galeria_arte.cpp

# Regra padrão: compilação do arquivo único concatenado para entrega
all: $(TARGET)

$(TARGET): $(SINGLE_SRC)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build $(TARGET)

# Alvo opcional: compilação da versão modular (src/)
modular:
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -Isrc -c src/camera.cpp -o build/camera.o
	$(CXX) $(CXXFLAGS) -Isrc -c src/lighting.cpp -o build/lighting.o
	$(CXX) $(CXXFLAGS) -Isrc -c src/object.cpp -o build/object.o
	$(CXX) $(CXXFLAGS) -Isrc -c src/scene.cpp -o build/scene.o
	$(CXX) $(CXXFLAGS) -Isrc -c src/hud.cpp -o build/hud.o
	$(CXX) $(CXXFLAGS) -Isrc -c src/main.cpp -o build/main.o
	$(CXX) $(CXXFLAGS) -o $(TARGET) build/*.o $(LIBS)

.PHONY: all run clean modular
