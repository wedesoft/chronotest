chronotest: chronotest.o
	gcc -o $@ $^

.c.o:
	gcc -c -Wall -Werror -o $@ $<
