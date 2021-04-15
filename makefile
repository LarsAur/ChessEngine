main: main.c fen.c utils.c moveHandler.c test.c search.c eval.c sorting.c hashing.c
	gcc -Og -o chess.out -Wall main.c fen.c utils.c moveHandler.c test.c search.c eval.c sorting.c hashing.c -lm -g -pg

.PHONY: run
run: 
	./chess.out

.PHONY: clean
clean:
	rm ./chess.out