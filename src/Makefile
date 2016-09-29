
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99
OBJS = main.o deamon.o files.o


EXEC = SyncDeamon.out

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $<

main.o: deamon.h
deamon.o: includes.h

clean:
	rm *.o

.PHONY: clean
