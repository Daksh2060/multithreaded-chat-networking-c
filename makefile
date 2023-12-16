all: chat

chat: 
	gcc -g -Wall -Wextra chat.c list.h list.o -o chat -lpthread

clean:
	rm -f chat