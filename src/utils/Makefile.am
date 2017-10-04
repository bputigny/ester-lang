EXTRA_DIST = log.hpp termcolor.hpp args.hpp

AM_CPPFLAGS = -I$(top_srcdir)/src/ir

noinst_LTLIBRARIES = libutils.la
libutils_la_SOURCES = utils.cpp

noinst_bindir = $(abs_top_builddir)/src
noinst_bin_PROGRAMS = test-utils
test_utils_SOURCES = main.cpp
test_utils_LDADD = libutils.la
