test: main.o thompson.o y.tab.o regexp.h
	gcc -o re main.o thompson.o y.tab.o
	./re test
y.tab.o: parse.y regexp.h 
	bison -y -v parse.y
	gcc -c y.tab.c
main.o: main.c regexp.h
	gcc -c main.c
thompson.o: thompson.c regexp.h
	gcc -c thompson.c
