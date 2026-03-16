CXX      := g++

CXXFLAGS := -O3 -march=native -std=c++17 -Wall

SRCS     := $(shell find . -name "*.cpp")

TARGET   := main

all:
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) `pkg-config --cflags --libs opencv4`

clean:
	rm -f $(TARGET)

.PHONY: all clean