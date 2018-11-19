#
# frontend of Makefile for kemacs
#

SHELL=/bin/sh
# CC=clang -Wno-comment # with LLVM Clang
CC=cc
# This value of $(CC) is used ONLY when tailoring makefile.
# If you want to change the compiler for making kemacs itself,
# modify mf.c instead.

MAKEFILE=make.file
CONFIG=econfig.h

MAKE="make" $(MFLAGS)

SED=LANG=C sed

all \
install \
clean \
realclean \
depend \
debug \
lint \
co \
inst-bin \
inst-man \
inst-lib:	$(MAKEFILE) FORCE
	      @ $(MAKE) -f $(MAKEFILE) $@

$(MAKEFILE):	mf.c mf.dep $(CONFIG)
	      @ echo "Tailoring makefile ..."
	      @ echo "## This file is made from mf.c" >$@
	      @ echo "## Don't modify this directly!" >>$@
	      @ echo "## Instead, modify mf.c and type" >>$@
	      @ echo "##	make $(MAKEFILE)" >>$@
	      @ echo "" >>$@
		
		if (echo ': # \'; echo exit 1) | $(CC) -E - | sh; then \
			$(CC) -E mf.c; \
		else \
			$(CC) -E -P mf.c; \
		fi | $(SED) -e '/^#/d' -e '/^[.	]*$$/d' \
			-e 's/^ /	/' >>$@
		cat mf.dep >>$@

FORCE:

.DEFAULT:;	co -l $@
