AC_PREREQ([2.65])
AC_INIT([Cainteoir Text-to-Speech], [0.8], [https://github.com/rhdunn/cainteoir-gtk/issues], [cainteoir-gtk], [https://github.com/rhdunn/cainteoir-gtk])
AM_INIT_AUTOMAKE()

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES])
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR([src])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

LT_PREREQ([2.2.6])
LT_INIT(disable-static)

dnl ================================================================
dnl gettext support.
dnl ================================================================

IT_PROG_INTLTOOL([0.40.0])

AC_SUBST([GETTEXT_PACKAGE], [cainteoir-gtk])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Define to the gettext package name.])

dnl ================================================================
dnl Program checks.
dnl ================================================================

AC_PROG_CXX
AC_PROG_MAKE_SET

dnl ================================================================
dnl C++11 support
dnl ================================================================

CXX11_CHECK_CXX11_SUPPORT

CXX11_NULLPTR
CXX11_DECLTYPE
CXX11_AUTO
CXX11_RANGE_BASED_FOR
CXX11_BRACE_INITIALIZATION
CXX11_SCOPED_ENUMS
CXX11_CONSTEXPR

CXX11_REQUIRE_STD_INITIALIZER_LIST
CXX11_REQUIRE_STD_SHARED_PTR

CXX11_REQUIRE_STD_BEGIN
CXX11_REQUIRE_STD_END

dnl ================================================================
dnl getopt checks.
dnl ================================================================

AC_CHECK_HEADERS([getopt.h])
AC_CHECK_FUNCS([getopt_long])

dnl ================================================================
dnl GTK+ checks.
dnl ================================================================

PKG_CHECK_MODULES([GTK], [gtk+-3.0 >= 3.4.0])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

dnl ================================================================
dnl sigc++ checks.
dnl ================================================================

PKG_CHECK_MODULES([SIGCXX], [sigc++-2.0])
AC_SUBST(SIGCXX_CFLAGS)
AC_SUBST(SIGCXX_LIBS)

dnl ================================================================
dnl Generate output.
dnl ================================================================

AC_CONFIG_FILES([
	Makefile
	po/Makefile.in
])
AC_OUTPUT

AC_MSG_NOTICE([

    Configuration for Cainteoir Text-to-Speech Gtk front-end complete.

        Source code location:          ${srcdir}
        Compiler:                      ${CXX}
        Compiler flags:                ${CXXFLAGS}
])
