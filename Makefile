CCFLAGS = $(shell pkg-config --cflags glfw3 glew eigen3)
LDFLAGS = $(shell pkg-config --libs glfw3 glew eigen3) -lChronoEngine

chronotest: chronotest.o
	g++ -o $@ $^ $(LDFLAGS)

.cc.o:
	g++ -c -Wall -Werror $(CCFLAGS) -o $@ $<
