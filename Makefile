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

$(BIN): $(BINDIR) ddmin main runner
	$(CC) $(OBJS) -o $(BIN)

$(BINDIR):
	mkdir -p $(BINDIR)

ddmin : src/ddmin.c
	$(CC) -c -o bin/ddmin.o src/ddmin.c

runner : src/runner.c
	$(CC) -c -o bin/runner.o src/runner.c

main : src/main.c
	$(CC) -c -o bin/main.o src/main.c
clean:
	rm -rf $(BINDIR)
