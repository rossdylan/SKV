CC=clang
CFLAGS= -Wall -Wextra -Werror -pedantic -O2 -pipe -march=native
DEBUG_CFLAGS= -Wall -Wextra -Werror -pedantic -O0 -g -pipe -DDEBUG_MSG_ENABLE
INCLUDE= -I../include
LFLAGS = -L../ -locto

.PHONY: all
all: test_disk_tree test_mem_btree test_thing_derp
	./test_disk_tree
	./test_mem_btree
	./test_think_derp

test_disk_tree: test_disk_tree.c
	$(CC) $(INCLUDE) -o test_disk_tree $(CFLAGS) test_disk_tree.c $(LFLAGS)

test_mem_btree: test_mem_btree.c
	$(CC) $(INCLUDE) -o test_mem_btree $(CFLAGS) test_mem_btree.c $(LFLAGS)

test_thing_derp:
	$(CC) $(INLCUDE) -o test_thing_derp $(CLFAGS) test_thing_derp.c $(LFLAGS)

.PHONY: clean
clean:
	rm -f *.o
	rm -f test_disk_tree
	rm -f test_mem_btree
	rm -f test_thing_derp
