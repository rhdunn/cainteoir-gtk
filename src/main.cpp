/* Cainteoir Gtk Application.
 *
 * Copyright (C) 2011 Reece H. Dunn
 *
 * This file is part of cainteoir-gtk.
 *
 * cainteoir-gtk is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cainteoir-gtk is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cainteoir-gtk.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <gtkmm.h>
#include <cainteoir/platform.hpp>
#include <cainteoir/engines.hpp>
#include <cainteoir/document.hpp>
#include <cainteoir/languages.hpp>
#include <locale.h>

namespace rdf = cainteoir::rdf;
namespace tts = cainteoir::tts;

#include "settings.hpp"
#include "document.hpp"
#include "voice_selection.hpp"
#include "metadata.hpp"
#include "cainteoir.hpp"

int main(int argc, char ** argv)
{
	cainteoir::initialise();

	setlocale(LC_MESSAGES, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	Gtk::Main app(argc, argv);

	Cainteoir window(argc > 1 ? argv[1] : NULL);
	Gtk::Main::run(window);

	cainteoir::cleanup();
	return 0;
}
