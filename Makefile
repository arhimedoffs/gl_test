TARGET = gl_configurator
TARGET_LIB = libconfig.so

# Configuration filename
CONFFILE = sp.cfg

INCLUDES = -Isrc/include

CC = gcc
RM = rm -f

SOURCES = src/main.c src/config.c
OBJS = $(subst src,build,$(SOURCES:.c=.o))
DEPS = $(OBJS:.o=.d)

CFLAGS = -Wall -O2 -g -fPIC -DCONF_FILE=$(CONFFILE)
LDFLAGS = 

RUNPREFIX = LD_LIBRARY_PATH=./

.PHONY: all
all: build $(TARGET)

build:
	mkdir -p $@

$(TARGET_LIB): build/config.o
	$(CC) -shared ${LDFLAGS} -o $@ $^

$(TARGET): build/main.o $(TARGET_LIB)
	$(CC) ${LDFLAGS} -L. -o $@ build/main.o -lconfig

build/config.d: src/config.c build
	$(CC) -c $(INCLUDES) $(CFLAGS) -MM -MT $(@:.d=.o) $< -o $@

build/main.d: src/main.c build
	$(CC) -c $(INCLUDES) $(CFLAGS) -MM -MT $(@:.d=.o) $< -o $@

include $(DEPS)

build/%.o: $|
	$(CC) -c -o $@ $< $(INCLUDES) $(CFLAGS)


# build/config.o: src/config.c src/include/config.h
# 	$(CC) -c $(INCLUDES) $(CFLAGS) src/config.c -o $@

# build/main.o: src/main.c src/include/config.h
# 	$(CC) -c $(INCLUDES) $(CFLAGS) -DCONF_FILE=$(CONFFILE) src/main.c -o $@

.PHONY: run
run: build $(TARGET)
	$(RUNPREFIX) ./$(TARGET)

test: build $(TARGET)
	$(RUNPREFIX) ./$(TARGET) add "0/0" "name1" "value1"
	$(RUNPREFIX) ./$(TARGET) add "0/1-0/3" "nameRange" "valueRange"
	$(RUNPREFIX) ./$(TARGET) add "1/1-2/2" "nameRange2" "valueRange2"
	$(RUNPREFIX) ./$(TARGET) add "0/2" "nameRange" "valueNew"
	$(RUNPREFIX) ./$(TARGET) get "0/2" "nameRange"
	$(RUNPREFIX) ./$(TARGET) get "0/0" "nameNotExist"
	$(RUNPREFIX) ./$(TARGET) get "2/2" "nameSome"
	$(RUNPREFIX) ./$(TARGET) del "0/3" "nameRange"
	$(RUNPREFIX) ./$(TARGET) del "0/0" "nameNotExist"
	$(RUNPREFIX) ./$(TARGET) del "2/2" "nameSome"

.PHONY: clean
clean:
	echo $(DEPS)
	-$(RM) $(TARGET) $(TARGET_LIB) $(OBJS) $(DEPS)
	-$(RM) -d build
