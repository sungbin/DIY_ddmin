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

$(BIN): $(BINDIR) ddmin mymin main runner
	$(CC) $(OBJS) -g -o $(BIN)

$(BINDIR):
	mkdir -p $(BINDIR)

ddmin : src/ddmin.c
	$(CC) -c -g -o bin/ddmin.o src/ddmin.c

mymin : src/mymin.c
	$(CC) -c -g -o bin/mymin.o src/mymin.c

runner : src/runner.c
	$(CC) -c -g -o bin/runner.o src/runner.c

main : src/main.c
	$(CC) -c -g -o bin/main.o src/main.c

clean:
	rm -rf $(BINDIR)
