debug: 
	${CC} -I./header main.c source/stack.c  -DDEBUG -ggdb3 -O0 -Og -Wall -Wextra -Werror -o debug
