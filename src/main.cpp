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

#include <config.h>
#include <gtk/gtk.h>
#include <sigc++/signal.h>
#include <cainteoir/platform.hpp>

#include "cainteoir.hpp"

static void load_gtk3_theme(const std::string &theme)
{
	std::string theme_path = std::string(DATADIR "/" PACKAGE "/themes/") + theme;

	try
	{
		cainteoir::mmap_buffer theme(theme_path.c_str());

		GdkScreen *screen = gdk_display_get_default_screen(gdk_display_get_default());

		GtkCssProvider *provider = gtk_css_provider_new();
		gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (provider), theme.begin(), theme.size(), NULL);
	}
	catch (const std::exception &e)
	{
	}
}

int main(int argc, char ** argv)
{
	cainteoir::initialise();

	setlocale(LC_MESSAGES, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	gtk_init(&argc, &argv);

	char *theme_name = NULL;
	g_object_get(gtk_settings_get_default(), "gtk-theme-name", &theme_name, NULL);

	load_gtk3_theme("gtk3-common.css");
	load_gtk3_theme(std::string(theme_name) + "/gtk3.css");

	g_free(theme_name);

	Cainteoir window(argc > 1 ? argv[1] : NULL);

	gtk_main();
	cainteoir::cleanup();
	return 0;
}
