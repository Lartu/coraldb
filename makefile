# +=======================================+
# | The CoralDB key-value database system |
# +=======================================+

# +----------+
# | MAKEFILE |
# +----------+
# Usage:
# make

#PREFIX is environment variable, but if it is not set, then set default value
ifeq ($(PREFIX),)
PREFIX := /usr/local
endif

# --- Makefile Data ---
SOURCE = source/coraldb.cpp
BUILDDIR = build
OUT = $(BUILDDIR)/coraldb

# --- Compilation Flags ---
FLAGS = -Wall -std=gnu++11 -fpermissive -pthread

# --- Build Rules ---
# Build CoralDB
all:
	mkdir -p $(BUILDDIR)
	$(CXX) $(FLAGS) $(SOURCE) -o $(OUT)

# Delete built file
clean:
	rm -rf build

install: $(OUT)
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 775 $(OUT) $(DESTDIR)$(PREFIX)/bin/
	
uninstall:
	rm $(DESTDIR)$(PREFIX)/bin/$(OUT)
