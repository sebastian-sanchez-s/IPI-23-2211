# Commandline arguments:
# src = filename to compile without .c
# N   = number to pass to src

CC=gcc
CFLAGS=-std=c11

STAMP=$(src)_$M_$N

NCOLSTEX=5

all: $(src) run

$(src): $(src).c
	mkdir -p obj
	$(CC) $< $(CFLAGS) -o obj/$(src).out

pdf: raw/$(STAMP)
	mkdir -p tex
	mkdir -p pdf 
	chmod +x out2tex.sh
	./out2tex.sh $(NCOLSTEX) < raw/$(STAMP) > tex/$(STAMP).tex
	latexmk -quiet -pdf -auxdir="pdf/$(STAMP)" -outdir="pdf/$(STAMP)" tex/$(STAMP).tex

run: obj/$(src).out
	mkdir -p raw 
	./obj/$(src).out $M $N > raw/$(STAMP)
