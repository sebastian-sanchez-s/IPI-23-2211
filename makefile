# Flags
CC=gcc
WF=-Wall -Werror -Wpedantic
CF=-std=c11 -g -O2
LD=-I./cddlib/lib-src/ -L./cddlib/lib-src/.libs/ -pthread -lcdd

# Output organization
NCOLTEX=5
STAMP=m$(NCOL)n$(NROW)

main: $(addprefix obj/,main.o syt.o)
	$(CC) $(CF) $(WF) $^ $(LD) -o obj/a.out

obj/syt.o: syt.c
	mkdir -p obj
	$(CC) $(CF) $(WF) -I./cddlib/lib-src/ $< -c -o obj/$(basename $<).o

obj/main.o: main.c
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
