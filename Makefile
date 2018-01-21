build:
	gcc main.c -o edit -Wall -Wextra -pedantic -std=c99

run:
	make build && ./edit
