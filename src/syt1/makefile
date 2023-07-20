# Commandline arguments:
# M = NCOLS
# N = NROWS
src=syt1

CC=gcc
CFLAGS=-Wall -Werror -std=c11 -pthread

STAMP=$M_$N

all: $(src) run

$(src): $(src).c
	mkdir -p obj
	$(CC) $< $(CFLAGS) -o obj/$(src).out

run: obj/$(src).out
	mkdir -p raw 
	./obj/$(src).out $M $N
