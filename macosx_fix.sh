#!/bin/sh

set -e

tar jxf kemacs-2.1k.tar.bz2
tar jxf kemacs-2.1k.tar.bz2 kemacs-2.1k/{emacs.{HLP,TUT},Cstrings/cfromC.c}
mv kemacs-2.1k/emacs.HLP kemacs-2.1k/emacs.HLP_
mv kemacs-2.1k/emacs.TUT kemacs-2.1k/emacs.TUT_
mv kemacs-2.1k/Cstrings/cfromC.c kemacs-2.1k/Cstrings/cfromC_.c
tar jxf kemacs-2.1k.tar.bz2 kemacs-2.1k/{emacs.{hlp,tut},Cstrings/Cfromc.c}

for i in kemacs-2.1k/Cstrings/Makefile.Cstr kemacs-2.1k_patch*; do
	perl -i -pe 's/\b(cfromC)(.[co])\b/$1_$2/g' "$i"
done
for i in kemacs-2.1k/mf.c kemacs-2.1k_patch*; do
	perl -i -pe 's/(\.(HLP|TUT|[CH]))\b/$1_/g' "$i"
done

for i in kemacs-2.1k/*.[CH]; do		# econfig.h, mf.c should be excluded
	mv "$i" "$i"_
done

patch -p0 < kemacs-2.1k_patch1.unofficial
patch -p0 < kemacs-2.1k_patch2b.macosx
if [ -e kemacs-2.1k_patch3.local ]; then	# local use only.
	patch -p0 < kemacs-2.1k_patch3.local
fi

echo Done. Change directory to kemacs-2.1k, and type \`make\'.
