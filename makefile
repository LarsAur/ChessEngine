main: main.c fen.c utils.c moveHandler.c test.c search.c eval.c sorting.c hashing.c
	gcc -O3 -o chess.elf main.c fen.c utils.c moveHandler.c test.c search.c eval.c sorting.c hashing.c -lm

.PHONY: run
run: 
	./chess.elf

.PHONY: clean
clean:
	rm ./chess.elf