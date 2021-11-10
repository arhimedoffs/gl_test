TARGET = gl_configurator
TARGET_LIB = libconfig.so

INCLUDES = -Isrc/include

CC = gcc
RM = rm -f

SOURCES = src/main.c src/config.c
OBJS = $(subst src,build,$(SOURCES:.c=.o))

DEPS = $(OBJS:.o=.d)

CFLAGS = -Wall -O2 -g -fPIC
LDFLAGS =  

.PHONY: all
all: $(TARGET)

$(TARGET_LIB): build/config.o
	$(CC) -shared ${LDFLAGS} -o $@ $^

$(TARGET): build/main.o $(TARGET_LIB)
	$(CC) ${LDFLAGS} -L. -o $@ build/main.o -lconfig

build/config.d: src/config.c
	$(CC) -c $(INCLUDES) $(CFLAGS) -MMD -o build/config.o $^

build/main.d: src/main.c
	$(CC) -c $(INCLUDES) $(CFLAGS) -MMD -o build/main.o $^

#$(DEPS):%.d:%.c
#\t $(CC) $(CFLAGS) -MM $< >$@

include build/config.d
include build/main.d

.PHONY: run
run: $(TARGET)
	LD_LIBRARY_PATH=./ ./$(TARGET)

.PHONY: clean
clean:
	-$(RM) $(TARGET) $(TARGET_LIB) $(OBJS) $(DEPS)
