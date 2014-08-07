# cxx11.m4 serial 6
dnl Copyright (C) 2014 Reece H. Dunn
dnl
dnl This file is free software; Reece H. Dunn
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl ================================================================
dnl GtkHeaderBar
dnl ================================================================

AC_DEFUN([GTK3_HAVE_HEADER_BAR], [
	AC_LANG_PUSH(C)
	AC_MSG_CHECKING([if GtkHeaderBar is supported])
	OLD_CFLAGS=$CFLAGS
	CFLAGS="$CFLAGS `pkg-config --cflags gtk+-3.0`"
	OLD_LIBS=$LIBS
	LIBS="$LIBS `pkg-config --libs gtk+-3.0`"
	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM(
			[[#include <gtk/gtk.h>]],
			[[GtkWidget *header = gtk_header_bar_new();]])],
		[AC_MSG_RESULT([yes])
		 AC_DEFINE([HAVE_GTK3_HEADER_BAR], [1], [Define to 1 if GtkHeaderBar is supported])],
		[AC_MSG_RESULT([no])])
	CFLAGS=$OLD_CFLAGS
	LIBS=$OLD_LIBS
	AC_LANG_POP(C)])
