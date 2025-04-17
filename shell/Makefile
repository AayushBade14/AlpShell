#Compiler
CXX = g++

#DIRS
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

#Flags
CXXFLAGS = -Wall -g -std=c++11 -I$(INCLUDE_DIR)

LIBS = -lreadline

#Objs
OBJS = main.o $(SRC_DIR)/command.o $(SRC_DIR)/executor.o $(SRC_DIR)/history.o $(SRC_DIR)/job_control.o $(SRC_DIR)/lexer.o $(SRC_DIR)/parser.o $(SRC_DIR)/shell.o $(SRC_DIR)/util.o

#Target
TARGET = $(BUILD_DIR)/alpshell

#all-rule
all: $(TARGET)

#Linking
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(CXXFLAGS) $(LIBS)

#Compiling
main.o: $(INCLUDE_DIR)/lexer.h $(INCLUDE_DIR)/parser.h $(INCLUDE_DIR)/command.h $(INCLUDE_DIR)/executor.h $(INCLUDE_DIR)/job_control.h $(INCLUDE_DIR)/history.h
$(SRC_DIR)/shell.o: $(INCLUDE_DIR)/parser.h $(INCLUDE_DIR)/util.h $(INCLUDE_DIR)/shell.h
$(SRC_DIR)/parser.o: $(INCLUDE_DIR)/lexer.h $(INCLUDE_DIR)/parser.h
$(SRC_DIR)/executor.o: $(INCLUDE_DIR)/command.h $(INCLUDE_DIR)/executor.h
$(SRC_DIR)/command.o: $(INCLUDE_DIR)/command.h
$(SRC_DIR)/history.o: $(INCLUDE_DIR)/history.h
$(SRC_DIR)/job_control.o: $(INCLUDE_DIR)/job_control.h
$(SRC_DIR)/util.o: $(INCLUDE_DIR)/util.h
$(SRC_DIR)/lexer.o: $(INCLUDE_DIR)/lexer.h

# clean-rule
.PHONY: clean

clean:
	rm -rf $(OBJS) $(TARGET)
