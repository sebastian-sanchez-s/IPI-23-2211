# Flags
CC=gcc
WF=-Wall -Werror -Wpedantic
CF=-std=c11 -g -funroll-loops -O2 -fpic
LD=-L. -pthread -lqsopt -lm

SRCS=$(wildcard *.c)

# Output organization
NCOLTEX=5
STAMP=m$(NCOL)n$(NROW)

all: $(patsubst %.c,obj/%.o,$(SRCS))
	$(CC) $(CF) $(WF) $^ $(LD) -o obj/a.out

obj/%.o: %.c
	mkdir -p obj
	$(CC) $(CF) $(WF) $< -c -o obj/$(basename $<).o

cleanobj: obj/
	rm obj/*

run: obj/a.out
	mkdir -p raw
	./obj/a.out $(NCOL) $(NROW)

pdf: $(wildcard raw/$(STAMP)t*)
	mkdir -p tex
	mkdir -p pdf 
	chmod +x csv2tex.sh
	cat $^ | ./csv2tex.sh $(NCOLTEX) > tex/$(STAMP).tex
	latexmk -quiet -pdf -auxdir="pdf/$(STAMP)" -outdir="pdf/$(STAMP)" tex/$(STAMP).tex

cleanpdf: pdf/$(STAMP) tex/$(STAMP)
	rm -r $^
