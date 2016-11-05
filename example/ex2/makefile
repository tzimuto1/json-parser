CFLAGS=-Wall -g -DDEBUG

USER_DIR = ../..

all : $(USER_DIR)/libtson.a
	gcc -o tson tson.c $^ -lm

$(USER_DIR)/libtson.a :
	make -C $(USER_DIR)

.PHONY : clean
clean :
	-rm *.o tson


