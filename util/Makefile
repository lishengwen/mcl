SRCDIR=.
SRC:=$(wildcard $(SRCDIR)/*.c)
HEADER:=$(wildcard $(SRCDIR)/*.h)
OBJDIR=.
OBJ=$(addprefix $(OBJDIR)/,$(subst .c,.o,$(subst $(SRCDIR)/,,$(SRC))))
INCLUDEDIR=../include
LIBDIR=../lib
CC=gcc44
TARGET=libutil.a
CFLAGS=-g
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
	$(AR) $(ARFLAG) $(LFLAGS) $(TARGET) $(OBJ) 

after_process:
	-rm -f $(OBJDIR)/*.o
	-cp *.h $(INCLUDEDIR)/
	-cp $(TARGET) $(LIBDIR)/

clean:
	-rm -f .Dependencies
	-rm -f $(TARGET)
	-rm -f $(LIBDIR)/$(TARGET)

