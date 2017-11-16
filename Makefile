
main: osadnicy.c
	@gcc osadnicy.c -o osadnicy -std=c11 -pthread -Wall -Werror
	@echo "Kompilacja zakonczona"
run:
	@./osadnicy 25 10 10 35