/* Cainteoir Reader Application.
 *
 * Copyright (C) 2015 Reece H. Dunn
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
#include "i18n.h"

#include <gtk/gtk.h>
#include "reader_window.h"
#include <cainteoir-gtk/cainteoir_document_view.h>

#include <cainteoir/buffer.hpp>

static void
load_theme(const gchar *theme_name)
{
	try
	{
		const gchar *data_dir = getenv("CAINTEOIR_GTK_DATA_DIR");
		if (!data_dir)
			data_dir = DATADIR "/" PACKAGE;

		gchar *path = g_strdup_printf("%s/themes/%s", data_dir, theme_name);
		auto theme = cainteoir::make_file_buffer(path);
		g_free(path);

		GdkScreen *screen = gdk_display_get_default_screen(gdk_display_get_default());

		GtkCssProvider *provider = gtk_css_provider_new();
		gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_css_provider_load_from_data(GTK_CSS_PROVIDER (provider), theme->begin(), theme->size(), nullptr);
	}
	catch (const std::exception &e)
	{
	}
}

static void
on_window_destroy(GtkWidget *object, gpointer data)
{
	gtk_main_quit();
}

int
main(int argc, char ** argv)
{
	gtk_init(&argc, &argv);

	load_theme("gtk3-common.css");

	GtkWidget *window = reader_window_new((argc == 2) ? argv[1] : nullptr);
	g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), nullptr);

	gtk_widget_show_all(window);
	gtk_main();

	return 0;
}
