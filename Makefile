BINS := mean
CFLAGS := -std=c99 $(CFLAGS)
LDFLAGS := $(LDFLAGS)

all: $(BINS)

clean:
	-rm -f $(BINS)

