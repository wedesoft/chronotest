CCFLAGS = $(shell pkg-config --cflags glfw3 glew eigen3)
LDFLAGS = $(shell pkg-config --libs glfw3 glew eigen3) -lChronoEngine

all: chronotumble chronoorbit

chronotumble: chronotumble.o
	g++ -o $@ $^ $(LDFLAGS)

chronoorbit: chronoorbit.o
	g++ -o $@ $^ $(LDFLAGS)

clean:
	rm -f chronotumble chronoorbit *.o

.cc.o:
	g++ -c -Wall -Werror $(CCFLAGS) -o $@ $<
