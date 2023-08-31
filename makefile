#/****************************************************************************
# * Copyright (C) 2023 by Sebastián Sánchez                                                      
# *                                                                          
# *  Permission is hereby granted, free of charge, to any person obtaining
# *  a copy of this software and associated documentation files (the
# *  "Software"), to deal in the Software without restriction, including
# *  without limitation the rights to use, copy, modify, merge, publish,
# *  distribute, sublicense, and/or sell copies of the Software, and to
# *  permit persons to whom the Software is furnished to do so, subject to
# *  the following conditions:
# *  
# *  The above copyright notice and this permission notice shall be
# *  included in all copies or substantial portions of the Software.
# *  
# *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
# ****************************************************************************/

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
