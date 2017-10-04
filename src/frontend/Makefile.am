BUILT_SOURCES = parser.hpp parser.cpp scanner.cpp

EXTRA_DIST = frontend.hpp

AM_YFLAGS = -d
AM_CPPFLAGS = -I$(top_srcdir)/src/utils \
			  -I$(top_srcdir)/src/ir \
			  -I$(top_builddir)/src/

noinst_LTLIBRARIES = libparser.la
noinst_bindir = $(abs_top_builddir)
noinst_bin_PROGRAMS = ester-lang

libparser_la_SOURCES = parser.ypp scanner.lpp frontend.cpp
libparser_la_CXXFLAGS = $(AM_CXXFLAGS) \
						-Wno-deprecated-register
libscanner_la_CXXFLAGS = $(AM_CXXFLAGS) \
						-Wno-unneeded-internal-declaration
libparser_la_LIBADD = ../ir/libir.la

ester_lang_SOURCES = main.cpp
ester_lang_LDADD = ../utils/libutils.la \
				   ../ir/libir.la \
				   libparser.la
