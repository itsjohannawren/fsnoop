# Whether or not to show the full commands
MAKE_HIDE_CMD=yes

# Git version information
CC_DEFINES_GIT_SHA1=$(shell git rev-parse --verify --short=10 HEAD)
CC_DEFINES_GIT_BRANCH=$(shell git branch 2>/dev/null | grep -E '^\* ' | sed -e 's/^\* //')
CC_DEFINES_GIT_TAGS=$(shell git tag --contains | sort | paste -s -d + -)
CC_DEFINES_GIT_UNCLEAN=$(shell git status -s 2>/dev/null | wc -l)

# Compiler information and flags
CC=gcc
CC_FLAGS=-g -Wall -c -DGIT_SHA1="\"$(CC_DEFINES_GIT_SHA1)\"" -DGIT_BRANCH="\"$(CC_DEFINES_GIT_BRANCH)\"" -DGIT_TAGS="\"$(CC_DEFINES_GIT_TAGS)\"" -DGIT_UNCLEAN=$(CC_DEFINES_GIT_UNCLEAN)

# Linker information, flags, and libs
LD=ld
LD_FLAGS=
LD_LIBS=

# Other shell commands
RM=rm -f
CP=cp

# Source files
SRC_FILES=fsnoop.c resolve.c cache.c dll.c
# Automatic object files
OBJ_FILES=$(SRC_FILES:.c=.o)

# Linked executables
EXE_FILES=fsnoop

# Per executable linker flags and libs
EXE_fsnoop_FLAGS=-Bstatic -static
EXE_fsnoop_LIBS=
# Executable objects
EXE_fsnoop_OBJS=fsnoop.o resolve.o cache.o dll.o

# Magic
# =============================================================================
all: $(EXE_FILES)

clean:
	@echo "Cleaning ..."
	@echo "RM  $(EXE_FILES)"
	@$(RM) $(EXE_FILES)
	@echo "RM  *.o"
	@$(RM) *.o

.c.o:
	@if [ "$(MAKE_HIDE_CMD)" = "yes" ]; then \
		echo "CC  $<"; \
	else \
		echo "$(CC) $(CC_FLAGS) -o $@ $<"; \
	fi
	@$(CC) $(CC_FLAGS) -o $@ $< 2>&1 | sed -e 's/^/    /'

$(EXE_FILES): $(OBJ_FILES)
	@if [ "$(MAKE_HIDE_CMD)" = "yes" ]; then \
		echo "LD  $@"; \
	else \
		echo "$(CC) $(LD_FLAGS) $(LD_LIBS) $(EXE_$@_FLAGS) $(EXE_$@_LIBS) -o $@ $(EXE_$@_OBJS)"; \
	fi
	@$(CC) $(LD_FLAGS) $(LD_LIBS) $(EXE_$@_FLAGS) $(EXE_$@_LIBS) -o $@ $(EXE_$@_OBJS) 2>&1 | sed -e 's/^/    /'
