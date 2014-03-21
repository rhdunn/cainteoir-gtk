/* Cainteoir Gtk Application.
 *
 * Copyright (C) 2011-2012 Reece H. Dunn
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

#include "config.h"
#include "compatibility.hpp"
#include "i18n.h"

#include <sigc++/signal.h>
#include "cainteoir.hpp"

#include <cainteoir/path.hpp>

static cainteoir::path get_data_dir()
{
	const char *path = getenv("CAINTEOIR_GTK_DATA_DIR");
	if (path)
		return cainteoir::path(path);
	return cainteoir::path(DATADIR "/" PACKAGE);
}

static void load_gtk3_theme(const std::string &theme_name)
{
	try
	{
		auto theme = cainteoir::make_file_buffer(get_data_dir() / "themes" / theme_name);

		GdkScreen *screen = gdk_display_get_default_screen(gdk_display_get_default());

		GtkCssProvider *provider = gtk_css_provider_new();
		gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (provider), theme->begin(), theme->size(), nullptr);
	}
	catch (const std::exception &e)
	{
	}
}

int main(int argc, char ** argv)
{
	setlocale(LC_MESSAGES, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	gtk_init(&argc, &argv);

	char *theme_name = nullptr;
	g_object_get(gtk_settings_get_default(), "gtk-theme-name", &theme_name, nullptr);

	load_gtk3_theme("gtk3-common.css");
	load_gtk3_theme(std::string(theme_name) + "/gtk3.css");

	g_free(theme_name);

	Cainteoir window(argc > 1 ? argv[1] : nullptr);

	gtk_main();
	return 0;
}
