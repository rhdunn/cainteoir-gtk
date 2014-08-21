#!/bin/sh

LIBTOOLIZE=`which libtoolize`
if ! test -f "$LIBTOOLIZE" ; then
	# Mac OSX support ...
	LIBTOOLIZE=`which glibtoolize`
fi

mkdir -p m4
ln -sf README.md README
ln -sf docs/ReleaseNotes.md NEWS

run()
{
	echo "Running $@ ..."
	$@ || exit 1
}

run glib-gettextize --force --copy
run intltoolize --automake --copy --force
run ${LIBTOOLIZE} --copy --force
run aclocal -I m4
run autoheader
run automake --add-missing
run autoconf
