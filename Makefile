SRCDIR:=util net
TESTSUITE:=test
ALLDIR:=$(SRCDIR) $(TESTSUITE)
TARGET:=libmcl.a
AR=ar
ARFLAG=-rc
OBJDIR=.
LIBDIR=./lib
MAKEARGS=
SYSNAME=$(shell uname -sm)
SYSCOPY=$(shell uname -r)

####check sys type###
ifeq ($(findstring FreeBSD, $(SYSNAME)), FreeBSD)
	MAKEARGS+=-D_SYS_FREEBSD
	COPY_MACRO=-D_FREEBSD_RELEASE
endif

ifeq ($(findstring Linux, $(SYSNAME)), Linux)
	MAKEARGS+=-D_SYS_LINUX
	COPY_MACRO=-D_LINUX_LESS_2_4
endif

ifeq ($(findstring 2.6, $(SYSCOPY)), 2.6)
	COPY_MACRO=-D_LINUX_2_6
endif

ifeq ($(findstring 2.4, $(SYSCOPY)), 2.4)
	COPY_MACRO=-D_LINUX_2_4 
endif

MAKEARGS+=$(COPY_MACRO)

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
		(cd $$subdir && gmake 'MAKEARGS=$(MAKEARGS)') \
		done;

target:
	$(AR) $(ARFLAG) $(TARGET) $(OBJDIR)/*.o
	-mv $(TARGET) $(LIBDIR)/

test_suite:
	@for subdir in $(TESTSUITE); do \
		(cd $$subdir && gmake 'MAKEARGS=$(MAKEARGS)') \
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

