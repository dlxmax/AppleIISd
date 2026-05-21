# sim65 unit tests for AppleIISd firmware
#
# Requires cc65 (provides cl65, sim65). On this dev box it lives under
# /tmp/cc65 (see notes); override CC65_HOME / CC65_BIN to point elsewhere.

CC65_HOME ?= /tmp/cc65/usr/share/cc65
CC65_BIN  ?= /tmp/cc65/usr/bin
CL65      := $(CC65_BIN)/cl65
SIM65     := $(CC65_BIN)/sim65

export CC65_HOME

TESTS := test_smbase

.PHONY: all test clean
all: $(TESTS)

test_smbase: test_smbase.c smbase_decode.s
	$(CL65) -t sim65c02 -o $@ $^

test: $(TESTS)
	@set -e; for t in $(TESTS); do \
	  echo "=== $$t ==="; \
	  $(SIM65) ./$$t; \
	done

clean:
	rm -f $(TESTS) *.o
