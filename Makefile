CCFLAGS = $(shell pkg-config --cflags glfw3 glew eigen3)
LDFLAGS = $(shell pkg-config --libs glfw3 glew eigen3) -lChronoEngine

chronotumble: chronotumble.o
	g++ -o $@ $^ $(LDFLAGS)

clean:
	rm -f chronotumble *.o

.cc.o:
	g++ -c -Wall -Werror $(CCFLAGS) -o $@ $<
