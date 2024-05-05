SUBDIRS= src tests
EXTRA_DIST = doc/gssapi_mech.conf

dist-hook:
	rm -f $(distdir)/src/gssglue/gssapi/gssapi.h

pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA = libgssglue.pc

indent_sources = src/*.c src/*.h src/gssglue/gssapi/gssapi.h.in tests/*.c

indent:
	indent --original $(indent_sources)
	indent --original $(indent_sources)
	perl -pi -e 's/ $$//' $(indent_sources)
