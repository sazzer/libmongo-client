SUBDIRS		= src tests examples

ACLOCAL_AMFLAGS	= -I m4 --install
EXTRA_DIST	= NEWS README.rst

coverage:
	@echo "Making $@ in src"
	($(am__cd) src && $(MAKE) $(AM_MAKEFLAGS) $@)
