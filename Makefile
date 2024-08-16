chronotest: chronotest.o
	gcc -o $@ $^ -lglfw -lGLEW -lGL -lm

.c.o:
	gcc -c -Wall -Werror -o $@ $<
