
CCP = g++
CC = gcc
CCPFLAGS = -Wall
CCFLAGS = -Wall

SRCS = $(wildcard *.c)
BINDIR = bin

all: c_objs cpp_objs

c_objs: $(patsubst %.c, %, $(wildcard *.c))

cpp_objs: $(patsubst %.cpp, %, $(wildcard *.cpp))


%: %.c Makefile
	$(CC) -o $(BINDIR)/$@ $< $(CCFLAGS)

%: %.cpp Makefile
	$(CCP) -o $(BINDIR)/$@ $< $(CCPFLAGS)

clean:
	rm -f $(BINDIR)/*
