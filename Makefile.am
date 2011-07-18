AUTOMAKE_OPTIONS = subdir-objects

localedir = $(datadir)/locale

CAINTEOIR_CFLAGS = 
CAINTEOIR_LIBS   = -lcainteoir

AM_LDFLAGS  = \
	${CAINTEOIR_LIBS} \
	${GTK_LIBS} \
	${LTLIBINTL}

AM_CXXFLAGS = \
	${CAINTEOIR_CFLAGS}  \
	${GTK_CFLAGS}  \
	-std=c++0x -DDATADIR='"$(datadir)"' -DLOCALEDIR=\"$(localedir)\"

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS =
lib_LTLIBRARIES =
man1_MANS =
pkgdata_DATA =

noinst_bin_PROGRAMS =
noinst_LIBRARIES =

noinst_bindir =

EXTRA_DIST = config.rpath ChangeLog
CLEANFILES =
SUBDIRS = po

############################# libtool #########################################

EXTRA_DIST += config.guess config.sub ltmain.sh

# Increment if the interface has changed and is backward compatible
CURRENT=0

# Increment  if source files have changed
# Reset to 0 if the interface has changed
REVISION=0

# Increment  if the interface is backward compatible (superset)
# Reset to 0 if the interface is not backward compatible
AGE=0

LIBCAINTEOIR_VERSION=$(CURRENT):$(REVISION):$(AGE)

############################# gettext #########################################

check-gettext:
	@if test x$(USE_NLS) != "xyes" ; then echo "Missing gettext. Rerun configure and check for" \
	"'checking whether to use NLS... yes'!" ; exit 1 ; fi

update-po: check-gettext
	@find $(srcdir)/src/ -name "*.cpp" -print | sort > $(srcdir)/po/POTFILES.in.2 ; \
	if diff $(srcdir)/po/POTFILES.in $(srcdir)/po/POTFILES.in.2 >/dev/null 2>&1 ; then \
		rm -f $(srcdir)/po/POTFILES.in.2 ; \
	else \
		mv $(srcdir)/po/POTFILES.in.2 $(srcdir)/po/POTFILES.in ; \
	fi
	cd po && $(MAKE) $(AM_MAKEFLAGS) update-po

update-gmo: check-gettext
	cd po && $(MAKE) $(AM_MAKEFLAGS) update-gmo

force-update-gmo: check-gettext
	touch po/*.po
	cd po && $(MAKE) $(AM_MAKEFLAGS) update-gmo

force-update-gmo-%: check-gettext
	@language=`echo $@ | sed s/force-update-gmo-//` ; \
	if test ! -f po/$$language.po ; then echo "file po/$$language.po does not exist" ; exit 1 ; fi ; \
	touch po/$$language.po ; \
	cd po && $(MAKE) $(AM_MAKEFLAGS) update-gmo

.PHONY: check-gettext update-po update-gmo force-update-gmo

############################# ChangeLog #######################################

ChangeLog:
	git log > ChangeLog

dist-hook: ChangeLog

.PHONY: ChangeLog

EXTRA_DIST += ChangeLog

############################# applications ####################################

desktopdir = $(datadir)/applications
desktop_DATA = src/cainteoir-gtk.desktop

bin_PROGRAMS += src/cainteoir-gtk

src_cainteoir_gtk_SOURCES = \
	src/toc.hpp \
	src/toc.cpp \
	src/main.cpp

EXTRA_DIST += ${man1_MANS}

