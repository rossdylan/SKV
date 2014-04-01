CC=clang
CFLAGS= -Wall -Wextra -Werror -pedantic -O2 -pipe -march=native
#DEBUG_CFLAGS= -Wall -Wextra -Werror -pedantic -O0 -g -pipe -DDEBUG_MSG_ENABLE
INCLUDE= -I./include

# You make need to change this to '-fpic' if you're using a strange
# architecture like ancient SPARC or MIPS:
FPIC= -fPIC

# Archiver for building the static library:
AR=ar
ARFLAGS=rvs

# Default values for user-supplied compile time directives:
DEBUG_MSG=

# Enable debugging messages outside of the 'debug' target:
ifeq ($(DEBUG_MSG),y)
	CFLAGS += -DDEBUG_MSG_ENABLE
endif

.PHONY: all
all: libskv.a

libskv.a: btree.o disktree.o llist.o pagemanager.o
	$(AR) $(ARFLAGS) libskv.a btree.o disktree.o llist.o pagemanager.o

btree.o: src/btree.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $(FPIC) src/btree.c

disktree.o: src/disktree.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $(FPIC) src/disktree.c

llist.o: src/llist.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $(FPIC) src/llist.c

pagemanager.o: src/pagemanager.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $(FPIC) src/pagemanager.c

.PHONY: clean
clean:
	rm -f libskv.a
	rm -f *.o
