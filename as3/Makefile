CC=g++
CFLAGS=-c -Wall -O3 
LDFLAGS =-lGL -lGLU -lglut
SOURCES=as3.cpp BezPatch.cpp BezCurve.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=as3

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
