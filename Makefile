SRCDIR:=util
TESTSUITE:=test
ALLDIR:=$(SRCDIR) $(TESTSUITE)
TARGET:=libmcl.a
AR=ar
ARFLAG=-rc
OBJDIR=.
LIBDIR=./lib

#SRCDIR=src
#SRC:=$(wildcard $(SRCDIR)/*.c)
#OBJDIR=.
#OBJ=$(addprefix $(OBJDIR)/,$(subst .c,.o,$(subst $(SRCDIR)/,,$(SRC))))
#INCLUDEDIR=include
#CC=gcc44
#TARGET=mcl
#CFLAGS=-g
#OPTIMIZE=-O2

#INCLUDES=-I./include/ -I./ -I/user/local/include

#all:depend subdirs bin after_process
all:subdirs target test_suite after_process

#depend:
#	-rm -f tmpdepend
#	for i in $(SRCDIR)/*.c; do $(CC) $(INCLUDES) -MM $$i >>tmpdepend; done
#	sed -e "s!^[^ ]!$(OBJDIR)/&!" <tmpdepend >.Dependencies
#	-rm -f tmpdepend

#-include .Dependencies
#$(OBJDIR)/%.o: $(SRCDIR)/%.c
#	$(CC) $(CFLAGS) $(INCLUDES) $(OPTIMIZE) -o $@ -c $<

subdirs:
	@for subdir in $(SRCDIR); do \
		(cd $$subdir && gmake) \
		done;

target:
	$(AR) $(ARFLAG) $(TARGET) $(OBJDIR)/*.o
	-mv $(TARGET) $(LIBDIR)/

test_suite:
	@for subdir in $(TESTSUITE); do \
		(cd $$subdir && gmake) \
		done;

#bin:$(OBJ)
#	$(CC) $(LIBRARY) $(LFLAGS) $(OBJ) -o $(TARGET)

after_process:
	-rm -f $(OBJDIR)/*.o

clean:
	@for subdir in $(ALLDIR); do \
		(cd $$subdir && gmake clean) \
		done;
	-rm $(LIBDIR)/$(TARGET)
	-rm -f $(OBJDIR)/*.o

