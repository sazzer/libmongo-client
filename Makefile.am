SUBDIRS = src tests examples

ACLOCAL_AMFLAGS = -I m4 --install
EXTRA_DIST = NEWS

coverage:
	@echo "Making $@ in src"
	($(am__cd) src && $(MAKE) $(AM_MAKEFLAGS) $@)
