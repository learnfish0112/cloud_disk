Srcs := $(wildcard *.c)
Objs := $(patsubst %.c, %.o, $(Srcs))
Out  := server
CC   := gcc
CFLAGS := -g -Wall -lpthread


$(Out): $(Objs)
	$(CC) $^ -o $@ -lpthread 
%.o: %.c head.h
	$(CC) -c $< $(CFLAGS)


.PHONY: clean rebuild
clean:
	$(RM) $(Out) $(Objs)
rebuild: clean $(Out)
