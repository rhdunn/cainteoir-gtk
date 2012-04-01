AC_PREREQ([2.65])
AC_INIT([Cainteoir Text-to-Speech], [0.4], [msclrhd@gmail.com], [cainteoir-gtk], [https://github.com/rhdunn/cainteoir-gtk])
AM_INIT_AUTOMAKE()

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES])
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR([src])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.17)

LT_INIT

dnl ================================================================
dnl Program checks.
dnl ================================================================

AC_PROG_CXX
AC_PROG_MAKE_SET

dnl ================================================================
dnl C++11 support
dnl ================================================================

AC_LANG_PUSH(C++)
AC_MSG_CHECKING([if $CXX supports C++11 without any flags])
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
		[[template <typename T> struct right_angle_brackets {};]
		 [typedef right_angle_brackets<right_angle_brackets<int>> cpp11_check;]],
		[[]])],
	[AC_MSG_RESULT([yes])
	 have_cxx11_native=yes],
	[AC_MSG_RESULT([no])
	 have_cxx11_native=no])
AC_LANG_POP(C++)

AC_LANG_PUSH(C++)
TEMP_CXXFLAGS="$CXXFLAGS"
CXXFLAGS="$CXXFLAGS -std=c++0x"
AC_MSG_CHECKING([if $CXX supports C++11 with the -std=c++0x flag])
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
		[[template <typename T> struct right_angle_brackets {};]
		 [typedef right_angle_brackets<right_angle_brackets<int>> cpp11_check;]],
		[[]])],
	[AC_MSG_RESULT([yes])
	 have_cxx11_cxx0x=yes],
	[AC_MSG_RESULT([no])
	 have_cxx11_cxx0x=no])
CXXFLAGS="$TEMP_CXXFLAGS"
AC_LANG_POP(C++)

AC_LANG_PUSH(C++)
TEMP_CXXFLAGS="$CXXFLAGS"
CXXFLAGS="$CXXFLAGS -std=gnu++0x"
AC_MSG_CHECKING([if $CXX supports C++11 with the -std=gnu++0x flag])
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
		[[template <typename T> struct right_angle_brackets {};]
		 [typedef right_angle_brackets<right_angle_brackets<int>> cpp11_check;]],
		[[]])],
	[AC_MSG_RESULT([yes])
	 have_cxx11_gnuxx0x=yes],
	[AC_MSG_RESULT([no])
	 have_cxx11_gnuxx0x=no])
CXXFLAGS="$TEMP_CXXFLAGS"
AC_LANG_POP(C++)

AC_MSG_CHECKING([if $CXX supports C++11])
if test "$have_cxx11_cxx0x" = yes ; then
	AC_MSG_RESULT([-std=c++0x])
	CXXFLAGS="$CXXFLAGS -std=c++0x"
elif test "$have_cxx11_gnuxx0x" = yes ; then
	AC_MSG_RESULT([-std=gnu++0x])
	CXXFLAGS="$CXXFLAGS -std=gnu++0x"
elif test "$have_cxx11_native" = yes ; then
	AC_MSG_RESULT([yes])
else
	AC_MSG_RESULT([no])
	AC_MSG_ERROR([C++11 is not supported by $CXX.])
fi

dnl ================================================================
dnl getopt checks.
dnl ================================================================

AC_CHECK_HEADERS([getopt.h])
AC_CHECK_FUNCS([getopt_long])

dnl ================================================================
dnl GTK+ checks.
dnl ================================================================

PKG_CHECK_MODULES([GTK], [gtk+-3.0 >= 3.0.0])
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

AC_CONFIG_FILES([Makefile po/Makefile.in])
AC_OUTPUT

AC_MSG_NOTICE([

    Configuration for Cainteoir Text-to-Speech Gtk front-end complete.

        Source code location:          ${srcdir}
        Compiler:                      ${CXX}
        Compiler flags:                ${CXXFLAGS}
])
