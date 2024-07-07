CC=gcc
CCOPTS=--std=gnu99 -Wall -D_LIST_DEBUG_ 
AR=ar

OBJS=bit_map.o\
     buddy_allocator.o\
	 pseudo_malloc.o

HEADERS=bit_map.h buddy_allocator.h pseudo_malloc.h

LIBS=libbuddy.a

BINS= pseudo_malloc_test

.phony: clean all

all:	$(LIBS) $(BINS)

%.o:	%.c $(HEADERS)
	$(CC) $(CCOPTS) -c -o $@  $<

libbuddy.a: $(OBJS) 
	$(AR) -rcs $@ $^
	$(RM) $(OBJS)

pseudo_malloc_test: pseudo_malloc_test.o $(LIBS)
	$(CC) $(CCOPTS) -o $@ $^ -lm

clean:
	rm -rf *.o *~ $(LIBS) $(BINS)