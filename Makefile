all: main_cpp main_chs main

CXX_LIBS = -lGL -lGLU -lglut
CXX = g++
CXX_SRC = src/main.cpp
CHS_SRC = src/main.chs

main_cpp: $(CXX_SRC)
	$(CXX) $(CXX_SRC) -o main_cpp $(CXX_LIBS)

main_chs: $(CHS_SRC)
	chs build $(CHS_SRC) -o main_chs

main:
	cargo build

clean:
	rm main_chs main_cpp
	chs clear
