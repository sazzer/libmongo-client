SUBDIRS = src tests examples

ACLOCAL_AMFLAGS = -I m4 --install

full-check: all
	@echo "Making $@ in tests"
	@${MAKE} ${AM_MAKE_FLAGS} -C tests $@
