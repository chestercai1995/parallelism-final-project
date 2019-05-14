
CCP = g++
CC = gcc
CCPFLAGS = -Wall -lpapi -std=c++11 -static
CCFLAGS = -Wall -lpapi

SRCS = $(wildcard *.c)
BINDIR = bin
INCLUDE = -I/misc/pool6/aniket/parallelism-final-project/papi-5.6.0/src
LIBS = /misc/pool6/aniket/parallelism-final-project/papi-5.6.0/src

all: c_objs cpp_objs
	./cp.sh

c_objs: $(patsubst %.c, %, $(wildcard *.c))

cpp_objs: $(patsubst %.cpp, %, $(wildcard *.cpp))


%: %.c Makefile
	$(CC) $(INCLUDE) -L$(LIBS) -o $(BINDIR)/$@ $< $(CCFLAGS)

%: %.cpp Makefile
	$(CCP) $(INCLUDE) -L$(LIBS) -o $(BINDIR)/$@ $< $(CCPFLAGS)

clean:
	rm -f $(BINDIR)/*
