# Commandline arguments:
# src = filename to compile without .c
# N   = number to pass to src

CC=gcc
CFLAGS=-std=c11

STAMP="$(src)_$N"
NCOLS=4

all: $(src) run pdf

$(src): $(src).c
	mkdir -p obj
	$(CC) $< $(CFLAGS) -o obj/$(src).out

pdf: raw/$(src)_$N 
	mkdir -p tex
	mkdir -p pdf 
	chmod +x out2tex.sh
	./out2tex.sh $(NCOLS) < raw/$(STAMP) > tex/$(STAMP).tex
	latexmk -pdf -auxdir="pdf/$(STAMP)" -outdir="pdf/$(STAMP)" tex/$(STAMP).tex

run: obj/$(src).out
	mkdir -p raw 
	./obj/$(src).out $N > raw/$(STAMP)
