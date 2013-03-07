SRCDIR=.
SRC:=$(wildcard $(SRCDIR)/*.c)
HEADER:=$(wildcard $(SRCDIR)/*.h)
OBJDIR=../
OBJ=$(addprefix $(OBJDIR)/,$(subst .c,.o,$(subst $(SRCDIR)/,,$(SRC))))
INCLUDEDIR=../include
LIBDIR=../lib
CC=gcc44
CFLAGS=-g -Werror $(MAKEARGS)
OPTIMIZE=-O2
CURRENT_DIR=net

INCLUDES=-I. -I../include -I../include/util -I./ -I/user/local/include

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
	@(echo 'mcl net compile done...')

after_process:
	-rm -rf $(INCLUDEDIR)/$(CURRENT_DIR)
	-mkdir $(INCLUDEDIR)/$(CURRENT_DIR)
	-cp *.h $(INCLUDEDIR)/$(CURRENT_DIR)/

clean:
	-rm -f .Dependencies

