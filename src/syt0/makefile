# Commandline arguments:
# M = NCOLS
# N = NROWS
src=syt0

CC=gcc
CFLAGS=-Wall -Werror -std=c11

STAMP=$M_$N

NCOLSTEX=5

all: $(src) run

$(src): $(src).c
	mkdir -p obj
	$(CC) $< $(CFLAGS) -o obj/$(src).out

pdf: raw/$(STAMP)
	mkdir -p tex
	mkdir -p pdf 
	chmod +x ./../csv2tex.sh
	./../csv2tex.sh $(NCOLSTEX) < ./raw/$(STAMP) > ./tex/$(STAMP).tex
	latexmk -quiet -pdf -auxdir="pdf/$(STAMP)" -outdir="pdf/$(STAMP)" tex/$(STAMP).tex

run: obj/$(src).out
	mkdir -p raw 
	./obj/$(src).out $M $N > raw/$(STAMP)
