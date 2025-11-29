# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Target binary
TARGET = base

# Source files
SRC = base.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

install: $(TARGET)
	mkdir -p $(HOME)/Documents/pms/compiled
	cp $(TARGET) $(HOME)/Documents/pms/compiled/

clean:
	rm -f $(TARGET)

.PHONY: all install clean
