all: s-talk

s-talk: 
	gcc -g -Wall -Wextra s-talk.c list.h list.o -o s-talk -lpthread

clean:
	rm -f s-talk