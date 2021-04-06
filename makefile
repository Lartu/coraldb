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
SOURCE = coraldb.cpp
OUT = coraldb

# --- Compilation Flags ---
FLAGS = -Wall -std=gnu++11 -fpermissive -pthread

# --- Build Rules ---
# Build CoralDB
all:
	mkdir -p build
	$(CXX) $(FLAGS) $(SOURCE) -o build/$(OUT)

# Delete built file
clean:
	rm -rf build

install: build/$(OUT)
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 775 build/$(OUT) $(DESTDIR)$(PREFIX)/bin/
	
uninstall:
	rm $(DESTDIR)$(PREFIX)/bin/$(OUT)
