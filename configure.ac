AC_PREREQ([2.65])
AC_INIT([Cainteoir Text-to-Speech], [0.1], [msclrhd@gmail.com], [cainteoir-gtk], [https://github.com/rhdunn/cainteoir-gtk])
AM_INIT_AUTOMAKE()

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES])
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR([src])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.18)

LT_INIT

AC_ARG_WITH([gtk3], AS_HELP_STRING([--with-gtk3], [Build with the Gnome 3 libraries.]))

dnl ================================================================
dnl Program checks.
dnl ================================================================

AC_PROG_CXX
AC_PROG_MAKE_SET

dnl ================================================================
dnl getopt checks.
dnl ================================================================

AC_CHECK_HEADERS([getopt.h])
AC_CHECK_FUNCS([getopt_long])

dnl ================================================================
dnl GTK+/Gtkmm checks.
dnl ================================================================

AS_IF([test "x$with_gtk3" = "xyes"], [
    PKG_CHECK_MODULES([GTK], [gtkmm-3.0 >= 3.0.0])
    gtkver="3.0"
], [
    PKG_CHECK_MODULES([GTK], [gtkmm-2.4 >= 2.10.0])
    gtkver="2.4"
])

AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

dnl ================================================================
dnl Generate output.
dnl ================================================================

AC_CONFIG_FILES([Makefile po/Makefile.in])
AC_OUTPUT

AC_MSG_NOTICE([

    Configuration for Cainteoir Text-to-Speech Gtk front-end complete.

        GTK+/Gnome Library version     ${gtkver}

        Source code location:          ${srcdir}
        Compiler:                      ${CXX}
        Compiler flags:                ${CXXFLAGS}
])
