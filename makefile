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

NCOLTEX=5
STAMP=m$(NCOL)n$(NROW)

all: $(OBJS) obj/consumer.o
	$(CC) $(CF) $(WF) $(OBJS) -o obj/a.out

obj/consumer.o: consumer.c table.c
	$(CC) $(CF) $(WF) $^ $(LD) -o $@

obj/%.o: %.c
	mkdir -p obj
	$(CC) $(CF) $(WF) $< -c -o obj/$(basename $<).o

clean: 
	rm obj/*
	rm raw/*

run: obj/a.out
	mkdir -p raw
	./obj/a.out $(NCOL) $(NROW)
	cat ./raw/Nc$(NCOL)r$(NROW)* > banned/c$(NCOL)r$(NROW)
	cat ./raw/Pc$(NCOL)r$(NROW)* > feasible/c$(NCOL)r$(NROW)

pdf: $(wildcard raw/$(STAMP)t*)
	mkdir -p tex
	mkdir -p pdf 
	chmod +x csv2tex.sh
	cat $^ | ./csv2tex.sh $(NCOLTEX) > tex/$(STAMP).tex
	latexmk -quiet -pdf -auxdir="pdf/$(STAMP)" -outdir="pdf/$(STAMP)" tex/$(STAMP).tex

cleanpdf: pdf/$(STAMP) tex/$(STAMP)
	rm -r $^
