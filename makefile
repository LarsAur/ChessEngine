main: main.c fen.c utils.c moveHandler.c test.c search.c eval.c sorting.c hashing.c book.c
	gcc -O3 -o chess.out -Wall main.c fen.c utils.c moveHandler.c test.c search.c eval.c sorting.c hashing.c book.c -lm

.PHONY: run
run: 
	./chess.out

.PHONY: mem
mem: 
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose ./chess.out
.PHONY: clean
clean:
	rm ./chess.out

