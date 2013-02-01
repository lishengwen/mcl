ALLDIR:=test_string test_list

all:subdirs

subdirs:
	@for subdir in $(ALLDIR); do \
		( cd $$subdir && gmake) \
		done;

clean:
	@for subdir in $(ALLDIR); do \
		(cd $$subdir && gmake clean) \
		done;

