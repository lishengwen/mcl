ALLDIR:=string list hash array log net_kq

all:subdirs

subdirs:
	@for subdir in $(ALLDIR); do \
		( cd $$subdir && gmake 'MAKEARGS=$(MAKEARGS)') \
		done;

clean:
	@for subdir in $(ALLDIR); do \
		(cd $$subdir && gmake clean) \
		done;

