ACLOCAL_AMFLAGS = -I m4
AM_CXXFLAGS = -Wall -pedantic -Wno-long-long -O0 -ggdb

SUBDIRS = po

bin_PROGRAMS = matrixcalc
matrixcalc_SOURCES = src/gettext.h src/matrixcalc.cpp \
	src/parser.cpp src/parser.h src/parseexpr.cpp src/parseexpr.h \
	src/matrix.cpp src/matrix.h src/tokenizer.h src/tokenizer.cpp \
	src/evalnodes.h src/evalnodes.cpp src/environ.h src/environ.cpp \
	src/value.h src/value.cpp
matrixcalc_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\"
matrixcalc_LDADD = $(LIBINTL)

dist_doc_DATA = LICENSE
EXTRA_DIST = build-aux/config.rpath Makefile.progtest.in \
	doc/prohlaseni.txt doc/zadani.txt \
	examples/dense_Linux_x86_64 examples/sparse_Linux_x86_64

## progtest distribution package
progtestname = @USERNAME@
progtestdistdir = $(builddir)/$(progtestname)

dist-progtest: distdir Makefile.progtest doc/Doxyfile
	-rm -rf $(progtestdistdir)
	mv $(distdir) $(progtestdistdir)
	mv $(progtestdistdir)/doc/*.txt $(progtestdistdir)
	cp -a Makefile.progtest $(progtestdistdir)/Makefile
	cp -a config.h.progtest $(progtestdistdir)/src/config.h
	sed -e 's|$(abs_top_srcdir)|.|g' \
		-e 's|\(^OUTPUT_DIRECTORY.*=\).*|\1|' \
		-e 's|\(^HTML_OUTPUT.*=\).*|\1 doc|' \
		doc/Doxyfile > $(progtestdistdir)/Doxyfile
	-rm -rf $(progtestdistdir)/{doc,po,m4,build-aux}
	-rm -f $(progtestdistdir)/{*.in,*.am,*.m4,configure*,ABOUT-NLS}

	-rm -f $(progtestname).zip
	zip -rq $(builddir)/$(progtestname).zip $(progtestdistdir)
	-rm -rf $(progtestdistdir)

include $(srcdir)/aminclude.am
MAINTAINERCLEANFILES = $(DX_CLEANFILES)
