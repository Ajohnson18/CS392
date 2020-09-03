# Alex Johnson - I pledge my honor that I have abided by the Stevens Honor System.

pfind: pfind.o pfinder.o
	gcc pfind.o pfinder.o -o pfind
pfind.o: pfind.c
	gcc -Wall -Werror -pedantic-errors -c pfind.c
pfinder.o: pfinder.c pfinder.h
	gcc -Wall -Werror -pedantic-errors -c pfinder.c
clean:
	rm -f *.o pfind pfind.exe