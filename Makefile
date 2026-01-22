# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Target binary
TARGET = pms

# Source files
SRC = pms.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

install: $(TARGET)
	mv -v $(TARGET) $(HOME)/Documents/pms/compiled/

clean:
	rm -f $(TARGET)

.PHONY: all install clean
