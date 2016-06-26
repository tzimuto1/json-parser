CFLAGS=-Wall -g
objects = parser.o json.o iterator.o

all : $(objects)
	gcc -o tson parser.o json.o iterator.o -lm

parser.o : parser.h

json.o : json.h

iterator.o : iterator.h

.PHONY : clean
clean :
	rm -fr *.o tson
