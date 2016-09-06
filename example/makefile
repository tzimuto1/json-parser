CFLAGS=-Wall -g -DDEBUG
objects = parser.o json.o iterator.o countries.o
USER_DIR = ..

all : $(objects)
	gcc -o countries countries.o parser.o json.o iterator.o -lm

countries.o : countries.c

parser.o : $(USER_DIR)/parser.h $(USER_DIR)/parser.c
	gcc -c $(USER_DIR)/parser.c -I $(USER_DIR)

json.o : $(USER_DIR)/json.h $(USER_DIR)/json.c
	gcc -c $(USER_DIR)/json.c -I $(USER_DIR)

iterator.o : $(USER_DIR)/iterator.h $(USER_DIR)/iterator.c
	gcc -c $(USER_DIR)/iterator.c -I $(USER_DIR)

.PHONY : clean
clean :
	-rm *.o countries
	