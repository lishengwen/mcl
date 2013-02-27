ALLDIR:=string list hash array

all:subdirs

subdirs:
	@for subdir in $(ALLDIR); do \
		( cd $$subdir && gmake) \
		done;

clean:
	@for subdir in $(ALLDIR); do \
		(cd $$subdir && gmake clean) \
		done;

