CFLAGS=-Wall -Wextra -Werror -g -pedantic
#-DDEBUG
objects = parser.o json.o iterator.o

all : libtson.a 

libtson.a : $(objects) utf8proc.o
	rm -fr libtson.a
	$(AR) rs libtson.a $^ 

parser.o : parser.h

json.o : json.h

iterator.o : iterator.h

utf8proc.o: utf8proc.h

# utf8proc/utf8proc.o : 
# 	make -C utf8proc utf8proc.o

.PHONY : clean all

clean :
	-rm -f *.o *.a
	