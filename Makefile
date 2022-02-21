CFLAGS = -Wall -std=c99

.PHONY: all
all: sfdpdec

sfdpdec: sfdpdec.c
	$(CC) $(CFLAGS) -o $@ $<
