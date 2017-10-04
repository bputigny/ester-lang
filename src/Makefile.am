EXTRA_DIST = path.hpp.in
BUILT_SOURCES = path.hpp

do_subst = sed -e 's,[@]abs_top_srcdir[@],$(abs_top_srcdir),g'

path.hpp: path.hpp.in Makefile
	$(do_subst) < $< > $@

SUBDIRS = . utils ir frontend

clean-local:
	rm -f path.hpp
