CCFLAGS = -DEIGEN_MAX_ALIGN_BYTES=32 $(shell pkg-config --cflags glfw3 glew eigen3)
LDFLAGS = $(shell pkg-config --libs glfw3 glew eigen3) -lChronoEngine

all: tumble orbit

tumble: tumble.o
	g++ -o $@ $^ $(LDFLAGS)

orbit: orbit.o
	g++ -o $@ $^ $(LDFLAGS)

clean:
	rm -f tumble orbit *.o

.cc.o:
	g++ -c -g -Wall -Werror $(CCFLAGS) -o $@ $<
