
CCP = g++
CC = gcc
CCPFLAGS = -Wall -lpapi
CCFLAGS = -Wall -lpapi

SRCS = $(wildcard *.c)
BINDIR = bin
INCLUDE = /opt/apps/papi/5.6.0/include
LIBS = /opt/apps/papi/5.6.0/lib

all: c_objs cpp_objs

c_objs: $(patsubst %.c, %, $(wildcard *.c))

cpp_objs: $(patsubst %.cpp, %, $(wildcard *.cpp))


%: %.c Makefile
	$(CC) -I$(INCLUDE) -L$(LIBS) -o $(BINDIR)/$@ $< $(CCFLAGS)

%: %.cpp Makefile
	$(CCP) -I$(INCLUDE) -L$(LIBS) -o $(BINDIR)/$@ $< $(CCPFLAGS)

clean:
	rm -f $(BINDIR)/*
