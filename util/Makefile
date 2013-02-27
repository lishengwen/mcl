SRCDIR=.
SRC:=$(wildcard $(SRCDIR)/*.c)
HEADER:=$(wildcard $(SRCDIR)/*.h)
OBJDIR=../
OBJ=$(addprefix $(OBJDIR)/,$(subst .c,.o,$(subst $(SRCDIR)/,,$(SRC))))
INCLUDEDIR=../include
LIBDIR=../lib
CC=gcc44
TARGET=libmclutil.a
CFLAGS=-g -Werror
OPTIMIZE=-O2
AR=ar
ARFLAG=-rc

INCLUDES=-I. -I../include -I./ -I/user/local/include

all:depend bin after_process

depend:
	-rm -f tmpdepend
	for i in $(SRCDIR)/*.c; do $(CC) $(INCLUDES) -MM $$i >>tmpdepend; done
	sed -e "s!^[^ ]!$(OBJDIR)/&!" <tmpdepend >.Dependencies
	-rm -f tmpdepend

-include .Dependencies
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTIMIZE) -o $@ -c $<

bin:$(OBJ)
	-echo 'mcl util compile done...'

after_process:
	-cp *.h $(INCLUDEDIR)/

clean:
	-rm -f .Dependencies

