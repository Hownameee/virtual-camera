CXX      := g++

CXXFLAGS := -O3 -flto -march=native -pipe -std=c++17 -Wall -Wextra

SRCS     := $(shell find . -name "*.cpp")
TARGET   := main
STB_DIR  := processors

all: 
	$(STB_DIR)/stb_image.h $(STB_DIR)/stb_image_write.h
	
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

	$(STB_DIR)/stb_image.h:
		@mkdir -p $(STB_DIR)
		wget -q -O $@ https://raw.githubusercontent.com/nothings/stb/master/stb_image.h

	$(STB_DIR)/stb_image_write.h:
		@mkdir -p $(STB_DIR)
		wget -q -O $@ https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

clean:
	rm -f $(TARGET)

.PHONY: all clean