CC      = gcc
INCLDIR = include/
BINDIR  = bin/
SRCDIR  = src/

_BIN    = main
BIN     = $(addprefix $(BINDIR), $(_BIN))

SRC     = $(wildcard src/*.c)
INCL    = $(wildcard include/*.h)
_OBJS   = $(patsubst src/%.c, %.o, $(SRC))
OBJS    = $(addprefix $(BINDIR), $(_OBJS))


all: $(BIN)

$(BIN): $(BINDIR) ddmin main runner range
	$(CC) $(OBJS) -g -o $(BIN) -lpthread -lm

$(BINDIR):
	mkdir -p $(BINDIR)

range : src/range.c
	$(CC) -c -g -o bin/range.o src/range.c

ddmin : src/ddmin.c
	$(CC) -c -g -o bin/ddmin.o src/ddmin.c

runner : src/runner.c
	$(CC) -c -g -o bin/runner.o src/runner.c

main : src/main.c
	$(CC) -c -g -o bin/main.o src/main.c

clean:
	rm -rf $(BINDIR)
