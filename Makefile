# Make the build silent by default
V =

ifeq ($(strip $(V)), )
	E = @echo
	Q = @
else
	E = @\#
	Q =
endif
export E Q

uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

PROGRAM = me

SRC = ansi.c basic.c buffer.c display.c ebind.c names.c \
	file.c fileio.c input.c isearch.c line.c lock.c main.c \
	pklock.c posix.c random.c region.c search.c spawn.c tcap.c \
	termio.c vt52.c window.c word.c globals.c version.c \
	usage.c wrapper.c utf8.c

OBJ = ansi.o basic.o buffer.o display.o ebind.o names.o \
	file.o fileio.o input.o isearch.o line.o lock.o main.o \
	pklock.o posix.o random.o region.o search.o spawn.o tcap.o \
	termio.o vt52.o window.o word.o globals.o version.o \
	usage.o wrapper.o utf8.o

HDR = edef.h efunc.h epath.h estruct.h version.h

CC = gcc
WARNINGS = -Wall -Wextra -Wstrict-prototypes -Wno-unused-parameter
#CFLAGS = -O0 $(WARNINGS) -g
CFLAGS = -O2 $(WARNINGS) -g
#CC = c89 +O3			# HP
#CFLAGS = -D_HPUX_SOURCE -DSYSV
#CFLAGS = -O4 -DSVR4		# Sun
#CFLAGS = -O -qchars=signed	# RS/6000
ifeq ($(uname_S), Linux)
DEFINES = -DAUTOCONF -DPOSIX -DUSG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE
endif

ifeq ($(uname_S), FreeBSD)
DEFINES = -DAUTOCONF -DPOSIX -DSYSV -D_FREEBSD_C_SOURCE -D_BSD_SOURCE \
	-D_SVID_SOURCE -D_XOPEN_SOURCE=600
endif

ifeq ($(uname_S), Darwin)
DEFINES = -DAUTOCONF -DPOSIX -DSYSV -D_DARWIN_C_SOURCE -D_BSD_SOURCE \
	-D_SVID_SOURCE -D_XOPEN_SOURCE=600
endif

#DEFINES = -DAUTOCONF

#LIBS = -ltermcap		# BSD
LIBS = -lcurses			# SYSV
#LIBS = -ltermlib
#LIBS = -L/usr/lib/termcap -ltermcap

LFLAGS = -hbx
BINDIR = /usr/bin
LIBDIR = /usr/lib

$(PROGRAM): $(OBJ)
	$(E) "	LINK	" $@
	$(Q) $(CC) $(LDFLAGS) $(DEFINES) -o $@ $(OBJ) $(LIBS)

clean:
	$(Q) rm -f $(PROGRAM) core *.o

install: $(PROGRAM)
	$(E) "	-> $(BINDIR)/$(PROGRAM)"
	$(Q) cp me $(BINDIR)
	$(Q) strip $(BINDIR)/$(PROGRAM)
	$(Q) chmod 755 $(BINDIR)/$(PROGRAM)
	$(Q) echo

.c.o:
	$(E) "	CC	" $@
	$(Q) $(CC) $(CFLAGS) $(DEFINES) -c $*.c

# Write the dependencies by hand to work on different make programs.

names.o: edef.h efunc.h estruct.h
ebind.o: edef.h efunc.h estruct.h
ansi.o: ansi.c estruct.h edef.h
basic.o: basic.c estruct.h edef.h
buffer.o: buffer.c estruct.h edef.h
display.o: display.c estruct.h edef.h utf8.h
file.o: file.c estruct.h edef.h
fileio.o: fileio.c estruct.h edef.h
input.o: input.c estruct.h edef.h
isearch.o: isearch.c estruct.h edef.h
line.o: line.c estruct.h edef.h
lock.o: lock.c estruct.h edef.h
main.o: main.c estruct.h efunc.h edef.h
pklock.o: pklock.c estruct.h
posix.o: posix.c estruct.h utf8.h
random.o: random.c estruct.h edef.h
region.o: region.c estruct.h edef.h
search.o: search.c estruct.h edef.h
spawn.o: spawn.c estruct.h edef.h
tcap.o: tcap.c estruct.h edef.h
termio.o: termio.c estruct.h edef.h
utf8.o: utf8.c utf8.h
vt52.o: vt52.c estruct.h edef.h
window.o: window.c estruct.h edef.h
word.o: word.c estruct.h edef.h
globals.o: estruct.h
