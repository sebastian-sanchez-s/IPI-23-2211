# Flags
CC=gcc
WF=-Wall -Werror -Wpedantic -Wextra
CF=-std=gnu11 -g3 -funroll-loops -O2 -fpic
LD=-L. -I./cddlib/lib-src/ -L./cddlib/lib-src/.libs/ -pthread -lcdd

SRCS=main.c\
		 producer.c\
		 queue.c\
		 table.c

OBJS=$(patsubst %.c,obj/%.o,$(SRCS))

all: $(OBJS) obj/consumer.o
	$(CC) $(CF) $(WF) $(OBJS) -o obj/a.out

obj/consumer.o: consumer.c table.c
	$(CC) $(CF) $(WF) $^ $(LD) -o $@

obj/%.o: %.c
	mkdir -p obj
	$(CC) $(CF) $(WF) $< -c -o obj/$(basename $<).o

cleanobj: 
	rm obj/*

cleandata:
	rm raw/*
	rm feasible/*
	rm banned/*

run: obj/a.out
	mkdir -p raw
	./obj/a.out $(NCOL) $(NROW)
	cat ./raw/Nc$(NCOL)r$(NROW)* > banned/c$(NCOL)r$(NROW)
	cat ./raw/Pc$(NCOL)r$(NROW)* > feasible/c$(NCOL)r$(NROW)
