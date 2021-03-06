AUTOMAKE_OPTIONS = subdir-objects

localedir = $(datadir)/locale

CAINTEOIR_CFLAGS = 
CAINTEOIR_LIBS   = -lcainteoir

AM_LDFLAGS  = \
	${CAINTEOIR_LIBS} \
	${GTK_LIBS} \
	${LTLIBINTL}

AM_CXXFLAGS = \
	-Isrc -Isrc/include \
	${CAINTEOIR_CFLAGS}  \
	${GTK_CFLAGS}  \
	-DDATADIR='"$(datadir)"' -DLOCALEDIR=\"$(localedir)\"

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
DISTCLEANFILES =
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

DISTCLEANFILES += ${INTLTOOL_FILES} po/.intltool-merge-cache

check-gettext:
	@if test x$(USE_NLS) != "xyes" ; then echo "Missing gettext. Rerun configure and check for" \
	"'checking whether to use NLS... yes'!" ; exit 1 ; fi

update-po: check-gettext
	@find $(srcdir)/src/ -name "*.cpp" -print | sed -e 's,\./,,g' | sort > $(srcdir)/po/POTFILES.in.2 ;
	@find $(srcdir)/data/ui/ -name "*.ui" -print | sed -e 's,\./,,g' -e 's,^,\[type: gettext/glade\],g' | sort >> $(srcdir)/po/POTFILES.in.2 ; \
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

############################# themes ##########################################

themedir = ${datadir}/cainteoir-gtk/themes

AdwaitaThemedir = ${themedir}/Adwaita
AdwaitaTheme_DATA = data/themes/Adwaita/gtk3.css

AmbianceThemedir = ${themedir}/Ambiance
AmbianceTheme_DATA = data/themes/Ambiance/gtk3.css

HighContrastThemedir = ${themedir}/HighContrast
HighContrastTheme_DATA = data/themes/HighContrast/gtk3.css

RadianceThemedir = ${themedir}/Radiance
RadianceTheme_DATA = data/themes/Radiance/gtk3.css

############################# libcainteoir-gtk ################################

libcainteoir_gtk_includedir = $(includedir)/cainteoir-gtk
libcainteoir_gtk_include_HEADERS = \
	src/include/cainteoir-gtk/cainteoir_audio_data_s16.h \
	src/include/cainteoir-gtk/cainteoir_document.h \
	src/include/cainteoir-gtk/cainteoir_document_index.h \
	src/include/cainteoir-gtk/cainteoir_document_view.h \
	src/include/cainteoir-gtk/cainteoir_metadata.h \
	src/include/cainteoir-gtk/cainteoir_settings.h \
	src/include/cainteoir-gtk/cainteoir_speech_parameter.h \
	src/include/cainteoir-gtk/cainteoir_speech_synthesizers.h \
	src/include/cainteoir-gtk/cainteoir_speech_voice_view.h \
	src/include/cainteoir-gtk/cainteoir_supported_formats.h \
	src/include/cainteoir-gtk/cainteoir_timebar.h \
	src/include/cainteoir-gtk/cainteoir_waveform_view.h

lib_LTLIBRARIES += src/libcainteoir-gtk/libcainteoir-gtk.la

src_libcainteoir_gtk_libcainteoir_gtk_la_SOURCES = \
	src/libcainteoir-gtk/cainteoir_audio_data_s16.cpp \
	src/libcainteoir-gtk/cainteoir_document.cpp \
	src/libcainteoir-gtk/cainteoir_document_index.cpp \
	src/libcainteoir-gtk/cainteoir_document_view.cpp \
	src/libcainteoir-gtk/cainteoir_metadata.cpp \
	src/libcainteoir-gtk/cainteoir_settings.cpp \
	src/libcainteoir-gtk/cainteoir_speech_parameter.cpp \
	src/libcainteoir-gtk/cainteoir_speech_synthesizers.cpp \
	src/libcainteoir-gtk/cainteoir_speech_voice_view.cpp \
	src/libcainteoir-gtk/cainteoir_supported_formats.cpp \
	src/libcainteoir-gtk/cainteoir_timebar.cpp \
	src/libcainteoir-gtk/cainteoir_waveform_view.cpp

############################# applications ####################################

desktopdir = $(datadir)/applications
desktop_DATA = src/cainteoir-gtk.desktop

bin_PROGRAMS += src/cainteoir-editor

src_cainteoir_editor_LDADD   = src/libcainteoir-gtk/libcainteoir-gtk.la
src_cainteoir_editor_SOURCES = \
	src/i18n.h \
	src/editor/main.cpp

bin_PROGRAMS += src/cainteoir-gtk

src_cainteoir_gtk_LDADD   = src/libcainteoir-gtk/libcainteoir-gtk.la
src_cainteoir_gtk_SOURCES = \
	src/i18n.h \
	src/reader/reader_application.h \
	src/reader/reader_application.cpp \
	src/reader/reader_document_view.h \
	src/reader/reader_document_view.cpp \
	src/reader/reader_preferences.h \
	src/reader/reader_preferences.cpp \
	src/reader/reader_settings_view.h \
	src/reader/reader_settings_view.cpp \
	src/reader/reader_window.h \
	src/reader/reader_window.cpp \
	src/reader/main.cpp

EXTRA_DIST += ${man1_MANS}

############################# tests ###########################################

noinst_bin_PROGRAMS += tests/content
tests_content_CXXFLAGS = ${AM_CXXFLAGS} -I src
tests_content_LDADD    = src/libcainteoir-gtk/libcainteoir-gtk.la
tests_content_SOURCES  = tests/content.cpp tests/tester.hpp

noinst_bin_PROGRAMS += tests/header_c_compat
tests_header_c_compat_CFLAGS  = ${AM_CXXFLAGS}
tests_header_c_compat_LDADD   = src/libcainteoir-gtk/libcainteoir-gtk.la
tests_header_c_compat_SOURCES = tests/header_c_compat.c

check: tests/content
	tests/content
