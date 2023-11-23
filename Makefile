SRCS := $(wildcard src/*.cpp) $(wildcard src/**/*.cpp)

TARGET := server

LIB := -lpthread -ldl

cc := gcc
CXX := g++

CXXFLAGS := -std=c++11 -Wall

all: lib/sqlite3.o $(TARGET).o

lib/sqlite3.o : lib/sqlite3.c
	$(cc) -o $@ -c $^ $(LIB) -nostartfiles

$(TARGET).o : $(SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@ lib/sqlite3.o -I./include/ $(LIB)

clean:
	rm -f $(TARGET).o lib/sqlite3.o