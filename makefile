CC=gcc
WFLAGS=-Wall -Werror -Wpedantic
CFLAGS=-std=c11 -pthread -O2

NCOLTEX=5
STAMP=m$(NCOL)n$(NROW)

main: main.c syt.c
	mkdir -p obj
	$(CC) syt.c $(CFLAGS) -c -o obj/syt.o
	$(CC) main.c $(CFLAGS) -c -o obj/main.o
	$(CC) obj/main.o obj/syt.o -o obj/a.out

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
