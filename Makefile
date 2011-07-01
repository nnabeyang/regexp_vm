test: main.o  pike.o thompson.o sub.o y.tab.o regexp.h
	gcc -o re main.o pike.o sub.o thompson.o y.tab.o
	./re test
y.tab.o: parse.y regexp.h 
	bison -y -v parse.y
	gcc -c y.tab.c
main.o: main.c regexp.h
	gcc -c main.c
thompson.o: thompson.c regexp.h
	gcc -c thompson.c
sub.o: sub.c regexp.h
	gcc -c sub.c
pike.o: pike.c regexp.h
	gcc -c pike.c
