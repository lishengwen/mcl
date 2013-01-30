SRCDIR=src
SRC:=$(wildcard $(SRCDIR)/*.c)
OBJDIR=.
OBJ=$(addprefix $(OBJDIR)/,$(subst .c,.o,$(subst $(SRCDIR)/,,$(SRC))))
INCLUDEDIR=include
CC=gcc44
TARGET=logsvr
CFLAGS=-g
OPTIMIZE=-O2

INCLUDES=-I./include/ -I./ -I/user/local/include

all:depend bin after_process

depend:
	-rm -f tmpdepend
	for i in $(SRCDIR)/*.c; do $(CC) $(INCLUDES) -MM $$i >>tmpdepend; done
	sed -e "s!^[^ ]!$(OBJDIR)/&!" <tmpdepend >.Dependencies
#	-rm -f tmpdepend

-include .Dependencies
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTIMIZE) -o $@ -c $<

bin:$(OBJ)
	$(CC) $(LIBRARY) $(LFLAGS) $(OBJ) -o $(TARGET)

after_process:
	-rm -f $(OBJDIR)/*.o

clean:
	-rm -f .Dependencies
	-rm -f $(TARGET)

