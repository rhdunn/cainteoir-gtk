# Cainteoir Text-to-Speech

[![Flattr this git repo](http://api.flattr.com/button/flattr-badge-large.png)](https://flattr.com/submit/auto?user_id=rhdunn&url=https://github.com/rhdunn/cainteoir-gtk&title=cainteoir-gtk&language=&tags=github&category=software)

- [Dependencies](#dependencies)
  - [Debian](#debian)
- [Building](#building)
- [Bugs](#bugs)
- [License Information](#license-information)

----------

The cainteoir-gtk project is a Gtk+/Gnome graphical front-end to the
cainteoir-engine library.

## Dependencies

In order to build Cainteoir Text-to-Speech, you need:

1.  a functional autotools system (`make`, `autoconf`, `automake`, `libtool`,
    `intltool` and `pkg-config`);
2.  a functional c++ compiler;
3.  the [Cainteoir text-to-speech engine](https://github.com/rhdunn/cainteoir-engine);
4.  Gtk+ 3.4 or later development libraries;
5.  SigC++ development libraries.

When running `cainteoir-gtk`, you will need:

1.  the `gnome-icon-theme-symbolic` package (to display the icons correctly).

### Debian

| Dependency                | Install                                                                   |
|---------------------------|---------------------------------------------------------------------------|
| autotools                 | `sudo apt-get install make autoconf automake libtool intltool pkg-config` |
| c++ compiler              | `sudo apt-get install gcc g++`                                            |
| gtk+                      | `sudo apt-get install libgtk-3-dev`                                       |
| sigc++                    | `sudo apt-get install libsigc++-2.0-dev`                                  |
| gnome-icon-theme-symbolic | `sudo apt-get install gnome-icon-theme-symbolic`                          |

## Building

Cainteoir Text-to-Speech uses a standard autotools build setup. Simply run:

    $ ./autogen.sh
    $ ./configure --prefix=/usr
    $ make
    $ sudo make install

## Bugs

Report bugs to the [cainteoir-gtk issues](https://github.com/rhdunn/cainteoir-gtk/issues)
page on GitHub.

## License Information

The Cainteoir Text-to-Speech Engine is released under the GPL version 3 or later license.

Cainteoir is a registered trademark of Reece Dunn.

W3C is a trademark (registered in numerous countries) of the World Wide Web Consortium; marks of W3C are registered and held by its host institutions MIT, ERCIM, and Keio.

All trademarks are property of their respective owners.
