FLAGS = -Wall -Wextra -g

build:
	gcc $(FLAGS) -o main main.c

run: build
	./main

