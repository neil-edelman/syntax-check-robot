# Makefile 1.2 (GNU Make 3.81; MacOSX gcc 4.2.1; MacOSX MinGW 4.3.0)

PROJ  := q1
VA    := 1
VB    := 0

# dirs
SDIR  := src
BDIR  := bin
BACK  := backup

# files in sdir
SRCS := $(wildcard $(SDIR)/*.c)
H    := $(wildcard $(SDIR)/*.h)
OBJS := $(patsubst $(SDIR)/%.c, $(BDIR)/%.o, $(SRCS))

CC   := gcc # /usr/local/i386-mingw32-4.3.0/bin/i386-mingw32-gcc javac nxjc
CF   := -Wall -Wextra -O3 -fasm -fomit-frame-pointer -ffast-math -funroll-loops -pedantic -std=c99 #-ansi # turn on -g for debugging and change -Og
OF   := # -framework OpenGL -framework GLUT

# props Jakob Borg and Eldar Abusalimov
EMPTY :=
SPACE := $(EMPTY) $(EMPTY)
ifeq (backup, $(firstword $(MAKECMDGOALS)))
  ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  BRGS := $(subst $(SPACE),_,$(ARGS))
  ifneq (,$(BRGS))
    BRGS := -$(BRGS)
  endif
  $(eval $(ARGS):;@:)
endif

######
# compiles the programme by default

default: $(BDIR)/$(PROJ)

# linking
$(BDIR)/$(PROJ): $(OBJS)
	$(CC) $(CF) $(OF) $(OBJS) -o $@

# compiling
$(OBJS): $(BDIR)/%.o: $(SDIR)/%.c $(H)
	@mkdir -p $(BDIR)
	$(CC) $(CF) -c $(SDIR)/$*.c -o $@

######
# phoney targets

.PHONY: setup clean backup

clean:
	-rm -f $(OBJS)

backup:
	@mkdir -p $(BACK)
	zip $(BACK)/$(INST)-`date +%Y-%m-%dT%H%M%S`$(BRGS).zip readme.txt Makefile $(SRCS) $(H)
	#git commit -am "$(ARGS)"
