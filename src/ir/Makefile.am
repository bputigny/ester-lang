EXTRA_DIST = ir.hpp solver.hpp

AM_CPPFLAGS = -I$(top_srcdir)/src/utils -I$(top_builddir)/src/

noinst_LTLIBRARIES = libir.la
libir_la_SOURCES = ast.cpp expr.cpp

noinst_bindir = $(abs_top_builddir)/src
noinst_bin_PROGRAMS = test-ir
test_ir_SOURCES = main.cpp
test_ir_LDADD = libir.la ../utils/libutils.la
