# 
PROG_NAME := sniffd
DS_DIR := data_strucuture/
PROG_SRCS := sniffd.c $(wildcard data_structure/*.c)
# PROG_SRCS := $(wildcard *.c)
PROG_OBJS := ${PROG_SRCS:.c=.o}
CLI_NAME := cli
CLI_SRCS := cli.c
CLI_OBJS := ${CLI_SRCS:.c=.o}
PROG_INCLUDE_DIRS := .
PROG_LIBRARY_DIRS :=
PROG_LIBRARIES :=

CFLAGS += $(foreach includedir, $(PROG_INCLUDE_DIRS), -I$(includedir))
LDFLAGS += $(foreach librarydir, $(PROG_LIBRARY_DIRS), -L$(libarydir))
LDFLAGS += $(foreach libarary, $(PROG_LIBRARIES), -l$(library))

.PHONY: all clean distclean PROG_NAME CLI_NAME

all: $(PROG_NAME) $(CLI_NAME)

$(PROG_NAME): $(PROG_OBJS)
	$(CC) $(PROG_OBJS) -o $(PROG_NAME)

$(CLI_NAME): $(CLI_OBJS)
	$(CC) $(CLI_OBJS) -o $(CLI_NAME)

clean:
	@- $(RM) $(PROG_NAME)
	@- $(RM) $(PROG_OBJS)
	@- $(RM) $(DS_DIR)*.o
	@- $(RM) $(CLI_NAME)
	@- $(RM) $(CLI_OBJS)

distclean: clean
