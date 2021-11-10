CC=gcc# C compiler
CONFFILE=sp2.cfg# Configuration filename
CFLAGS=-Wall -Wextra -O2 -g -DCONF_FILE=${CONFFILE}# C flags
LDFLAGS=# linking flags
RM=rm -f# rm command
TARGET=gl_configurator# target executable

TEST_FILE=sp.cfg

SRCS=main.c \
	config.c # source files
OBJS=$(SRCS:.c=.o)

.PHONY: all
all: ${TARGET}

$(TARGET): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CFLAGS) -MM $< >$@

include $(SRCS:.c=.d)

.PHONY: clean
clean:
	-${RM} ${TARGET} ${OBJS} $(SRCS:.c=.d) ${TEST_FILE}

.PHONY: run
run: ${TARGET}
	./${TARGET}

.PHONY: test
test: ${TARGET}
	./${TARGET} add "0/0" "name1" "value1"
	./${TARGET} add "0/1-0/3" "nameRange" "valueRange"
	./${TARGET} add "1/1-2/2" "nameRange2" "valueRange2"
	./${TARGET} add "0/2" "nameRange" "valueNew"
	./${TARGET} get "0/2" "nameRange"
	./${TARGET} get "0/0" "nameNotExist"
	./${TARGET} get "2/2" "nameSome"
	./${TARGET} del "0/3" "nameRange"
	./${TARGET} del "0/0" "nameNotExist"
	./${TARGET} del "2/2" "nameSome"

test_get: ${TARGET}
	./${TARGET} get "1/2" "name1"