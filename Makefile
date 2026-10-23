CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17

PKG_LIBS = $(shell pkg-config --libs glut glu gl 2>/dev/null)
PKG_CFLAGS = $(shell pkg-config --cflags glut glu gl 2>/dev/null)

ifeq ($(strip $(PKG_LIBS)),)
    LDLIBS = -lGL -lGLU -lglut -lm
else
    LDLIBS = $(PKG_LIBS) -lm
    CXXFLAGS += $(PKG_CFLAGS)
endif

TARGET = main
SRC_DIR = src
SRCS = $(SRC_DIR)/main.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
