TARGET = gl_configurator
TARGET_LIB = libconfig.so

# Configuration filename
CONFFILE = sp.cfg

INCLUDES = -Isrc/include

CC = gcc
RM = rm -f

SOURCES = src/main.c src/config.c
OBJS = $(subst src,build,$(SOURCES:.c=.o))

CFLAGS = -Wall -O2 -g -fPIC 
LDFLAGS = 

.PHONY: all
all: build $(TARGET)

build:
	mkdir -p $@

$(TARGET_LIB): build/config.o
	$(CC) -shared ${LDFLAGS} -o $@ $^

$(TARGET): build/main.o $(TARGET_LIB)
	$(CC) ${LDFLAGS} -L. -o $@ build/main.o -lconfig

build/config.o: src/config.c src/include/config.h
	$(CC) -c $(INCLUDES) $(CFLAGS) src/config.c -o $@

build/main.o: src/main.c src/include/config.h
	$(CC) -c $(INCLUDES) $(CFLAGS) -DCONF_FILE=$(CONFFILE) src/main.c -o $@

.PHONY: run
run: build $(TARGET)
	LD_LIBRARY_PATH=./ ./$(TARGET)

test: build $(TARGET)
	LD_LIBRARY_PATH=./ ./$(TARGET) add "0/0" "name1" "value1"
	LD_LIBRARY_PATH=./ ./$(TARGET) add "0/1-0/3" "nameRange" "valueRange"
	LD_LIBRARY_PATH=./ ./$(TARGET) add "1/1-2/2" "nameRange2" "valueRange2"
	LD_LIBRARY_PATH=./ ./$(TARGET) add "0/2" "nameRange" "valueNew"
	LD_LIBRARY_PATH=./ ./$(TARGET) get "0/2" "nameRange"
	LD_LIBRARY_PATH=./ ./$(TARGET) get "0/0" "nameNotExist"
	LD_LIBRARY_PATH=./ ./$(TARGET) get "2/2" "nameSome"
	LD_LIBRARY_PATH=./ ./$(TARGET) del "0/3" "nameRange"
	LD_LIBRARY_PATH=./ ./$(TARGET) del "0/0" "nameNotExist"
	LD_LIBRARY_PATH=./ ./$(TARGET) del "2/2" "nameSome"

.PHONY: clean
clean:
	-$(RM) $(TARGET) $(TARGET_LIB) $(OBJS)
	-$(RM) -d build
