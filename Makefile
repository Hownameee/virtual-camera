CXX      := g++

CXXFLAGS := -O3 -flto -march=native -pipe -std=c++17 -Wall -Wextra

SRCS     := $(shell find . -name "*.cpp")

TARGET   := main

all:
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) `pkg-config --cflags --libs opencv4`

clean:
	rm -f $(TARGET)

.PHONY: all clean