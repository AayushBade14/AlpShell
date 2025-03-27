#compilers
CXX = g++
CC = gcc

#dirs

SRC_DIR = ./src/
BUILD_DIR = ./build/
INCLUDE_DIR = ./include/

#flags
CXX_FLAGS = -Wall -g -I$(INCLUDE_DIR)

#srcs
SRCS = $(wildcard $(SRC_DIR)*.cpp)

#objs
OBJS = $(SRCS:$(SRC_DIR)%.cpp=$(BUILD_DIR)%.o)

#target
TARGET = $(BUILD_DIR)alpShell

#all-rule
all: $(TARGET)

#linking
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@

#compiling
$(BUILD_DIR)%.o: $(SRC_DIR)%.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@

#clean-rule
clean:
	rm -rf $(BUILD_DIR)*.o $(TARGET)
	echo "CLEANED!"

