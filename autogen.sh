#!/bin/sh

mkdir -p m4
ln -svf README.md README
ln -svf docs/ReleaseNotes.md NEWS

run()
{
	echo "Running $@ ..."
	$@ || exit 1
}

run glib-gettextize --force --copy
run intltoolize --automake --copy --force
run libtoolize --copy --force
run aclocal -I m4
run autoheader
run automake --add-missing
run autoconf
