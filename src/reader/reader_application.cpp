/* Cainteoir Reader application.
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

#include "reader_application.h"
#include "reader_window.h"

#include <cainteoir/buffer.hpp>
#include <locale.h>

G_DEFINE_TYPE(ReaderApplication, reader_application, GTK_TYPE_APPLICATION)

static void
load_theme(const gchar *theme_name, const gchar *theme_file)
{
	try
	{
		const gchar *data_dir = getenv("CAINTEOIR_GTK_DATA_DIR");
		if (!data_dir)
			data_dir = DATADIR "/" PACKAGE;

		gchar *path = nullptr;
		if (theme_name)
			path = g_strdup_printf("%s/themes/%s/%s", data_dir, theme_name, theme_file);
		else
			path = g_strdup_printf("%s/themes/%s", data_dir, theme_file);
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
reader_application_startup(GApplication *application)
{
	G_APPLICATION_CLASS(reader_application_parent_class)->startup(application);

	setlocale(LC_MESSAGES, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	load_theme(nullptr, "gtk3-common.css");

	gchar *theme_name = nullptr;
	g_object_get(gtk_settings_get_default(), "gtk-theme-name", &theme_name, nullptr);
	load_theme(theme_name, "gtk3.css");
	g_free(theme_name);
}

static void
reader_application_open(GApplication *application,
                        GFile **files,
                        gint n_files,
                        const gchar *hint)
{
	gchar *filename = n_files > 0 ? g_file_get_path(files[0]) : nullptr;

	GtkWidget *window = nullptr;
	GList *windows = gtk_application_get_windows(GTK_APPLICATION(application));
	if (windows)
	{
		window = GTK_WIDGET(windows->data);
		if (filename)
			reader_window_load_document(READER_WINDOW(window), filename);
	}
	else
		window = reader_window_new(GTK_APPLICATION(application), filename);
	gtk_widget_show_all(GTK_WIDGET(window));
	g_free(filename);

	gtk_window_present(GTK_WINDOW(window));
}

static void
reader_application_activate(GApplication *application)
{
	reader_application_open(application, nullptr, 0, nullptr);
}

static void
reader_application_class_init(ReaderApplicationClass *klass)
{
	G_APPLICATION_CLASS(klass)->startup = reader_application_startup;
	G_APPLICATION_CLASS(klass)->activate = reader_application_activate;
	G_APPLICATION_CLASS(klass)->open = reader_application_open;
}

static void
reader_application_init(ReaderApplication *application)
{
}

GApplication *
reader_application_new()
{
	gtk_init(nullptr, nullptr);

	g_set_application_name(i18n("Cainteoir Text-to-Speech"));
	return G_APPLICATION(g_object_new(READER_TYPE_APPLICATION,
		"application-id", "uk.co.reecedunn.cainteoir-gtk",
		"flags", G_APPLICATION_HANDLES_OPEN,
		nullptr));
}
