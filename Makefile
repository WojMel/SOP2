
main: osadnicy.c
	@gcc osadnicy.c -o osadnicy -std=c99 -pthread -Wall -Werror
	@echo "Kompilacja zakonczona"
run:
	@./osadnicy 10 11 3 7