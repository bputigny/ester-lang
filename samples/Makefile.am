EXTRA_DIST = poly1D.eq
BUILT_SOURCES = poly1D.cpp

if BUILD_SAMPLES
noinst_bindir = $(abs_top_builddir)
noinst_bin_PROGRAMS = poly1D
endif

poly1D_SOURCES = poly1D.cpp main-poly1D.cpp

poly1D.cpp: poly1D.eq ../src/frontend/ester-lang
	../src/frontend/ester-lang $< -o $@
