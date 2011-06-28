test: nfa.o y.tab.o regexp.h
	gcc -o nfa nfa.o y.tab.o
	./nfa test
y.tab.o: parse.y regexp.h 
	bison -y -v parse.y
	gcc -c y.tab.c
nfa.o: nfa.c regexp.h
	gcc -c nfa.c
