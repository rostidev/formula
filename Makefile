SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

CFLAGS = -std=c23 -pedantic -Wall -MMD -MP
LDFLAGS = -lSDL3 -lm

.PHONY: all clean

all: formula

formula: $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

-include $(OBJ:.o=.d)

clean:
	rm -rf *.o *.d formula
