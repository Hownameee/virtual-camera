CXX         := g++
CXXFLAGS    := -O3 -flto=auto -march=native -pipe -std=c++17
CXXDEVFLAGS := -O3 -flto=auto -march=native -pipe -std=c++17 -Wall -Wextra

SRCS        := $(shell find . -name "*.cpp")
TARGET      := main
STB_DIR     := image/deps

all: $(STB_DIR)/stb_image.h $(STB_DIR)/stb_image_write.h
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

dev: $(STB_DIR)/stb_image.h $(STB_DIR)/stb_image_write.h
	$(CXX) $(CXXDEVFLAGS) $(SRCS) -o $(TARGET)

# Dependencies
$(STB_DIR)/stb_image.h:
	@mkdir -p $(STB_DIR)
	wget -q -O $@ https://raw.githubusercontent.com/nothings/stb/master/stb_image.h

$(STB_DIR)/stb_image_write.h:
	@mkdir -p $(STB_DIR)
	wget -q -O $@ https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

clean:
	rm -f $(TARGET) $(STB_DIR)

.PHONY: all dev clean