all: hexeditplus

hexeditplus: hexeditplus.o 
	gcc -g -m32 -Wall -o hexeditplus hexeditplus.o

hexeditplus.o : task1c.c
	gcc -g -Wall -m32  -c -o hexeditplus.o  task1c.c
	
.PHONY: clean

clean: 
	rm -f *.o hexeditplus

