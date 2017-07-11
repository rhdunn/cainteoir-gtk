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
#include <cainteoir-gtk/cainteoir_document.h>
#include <cainteoir-gtk/cainteoir_settings.h>
#include <cainteoir-gtk/cainteoir_speech_synthesizers.h>

#include "reader_application.h"
#include "reader_window.h"
#include "reader_preferences.h"

#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "extensions/glib.h"

#if !GTK_CHECK_VERSION(3,12,0)
static void
gtk_application_set_accels_for_action(GtkApplication      *application,
                                      const gchar         *detailed_action_name,
                                      const gchar * const *accels)
{
	for (; *accels; ++accels) {
		gtk_application_add_accelerator(application, *accels, detailed_action_name, NULL);
	}
}
#endif

struct ReaderApplicationPrivate
{
	GApplication *self;
	GtkCssProvider *theme;

	CainteoirSettings *settings;
	CainteoirSpeechSynthesizers *tts;
	CainteoirDocument *doc;

	ReaderApplicationPrivate()
		: self(nullptr)
		, theme(gtk_css_provider_new())
		, settings(cainteoir_settings_new("settings.dat"))
		, doc(nullptr)
	{
		tts = cainteoir_speech_synthesizers_new(settings);
	}

	~ReaderApplicationPrivate()
	{
		g_object_unref(theme);
		g_object_unref(settings);
		g_object_unref(tts);
		if (doc) g_object_unref(doc);
	}
};

G_DEFINE_TYPE_WITH_PRIVATE(ReaderApplication, reader_application, GTK_TYPE_APPLICATION)

#define READER_APPLICATION_PRIVATE(object) \
	((ReaderApplicationPrivate *)reader_application_get_instance_private(READER_APPLICATION(object)))

GXT_DEFINE_TYPE_CONSTRUCTION(ReaderApplication, reader_application, READER_APPLICATION)

static GMenuModel *
create_application_menu()
{
	GMenu *menu = g_menu_new();

	g_menu_append(menu, i18n("_Preferences"), "app.preferences");
	g_menu_append(menu, i18n("_Quit"), "app.quit");

	return G_MENU_MODEL(menu);
}

static void
on_preferences_activated(GSimpleAction *action,
                         GVariant *parameter,
                         ReaderApplicationPrivate *priv)
{
	ReaderWindow *window = READER_WINDOW(gtk_application_get_active_window(GTK_APPLICATION(priv->self)));
	GtkWidget *prefs = reader_preferences_new(window, priv->settings, priv->tts, priv->doc);
	gtk_window_present(GTK_WINDOW(prefs));
}

static void
on_quit_activated(GSimpleAction *action,
                  GVariant *parameter,
                  ReaderApplicationPrivate *priv)
{
	g_application_quit(G_APPLICATION(priv->self));
}

typedef decltype(GActionEntry::activate) activate_fn;

static GActionEntry app_entries[] =
{
	{ "preferences", (activate_fn)on_preferences_activated, nullptr, nullptr, nullptr },
	{ "quit", (activate_fn)on_quit_activated, nullptr, nullptr, nullptr },
};

static void
on_theme_changed(GtkSettings *settings,
                 GParamSpec *param,
                 ReaderApplicationPrivate *priv)
{
	gchar *theme_name = nullptr;
	g_object_get(settings, "gtk-theme-name", &theme_name, nullptr);

	const gchar *data_dir = getenv("CAINTEOIR_GTK_DATA_DIR");
	if (!data_dir)
		data_dir = DATADIR "/" PACKAGE;

	gchar *path = g_strdup_printf("%s/themes/%s/gtk3.css", data_dir, theme_name);

	if (access(path, R_OK) == 0)
		gtk_css_provider_load_from_path(GTK_CSS_PROVIDER(priv->theme), path, nullptr);
	else
	{
		// Pass an empty CSS data string to clear the CSS provider styling ...
		gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(priv->theme), "\0", 0, nullptr);
	}

	bool is_kde = !strcmp(theme_name, "oxygen-gtk") // KDE4 L&F
	           || !strcmp(theme_name, "Orion");     // KDE5 L&F

	cainteoir_settings_set_boolean(priv->settings, "window", "have-csd", !is_kde);

	g_free(path);
	g_free(theme_name);
}

static void
reader_application_startup(GApplication *application)
{
	static const gchar *quit_accels[] = { "<Ctrl>Q", nullptr };

	G_APPLICATION_CLASS(reader_application_parent_class)->startup(application);
	ReaderApplicationPrivate *priv = READER_APPLICATION_PRIVATE(application);
	priv->self = application;

	setlocale(LC_MESSAGES, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	GdkScreen *screen = gdk_display_get_default_screen(gdk_display_get_default());
	gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(priv->theme), GTK_STYLE_PROVIDER_PRIORITY_USER);

	GtkSettings *settings = gtk_settings_get_default();
	g_signal_connect(settings, "notify::gtk-theme-name", G_CALLBACK(on_theme_changed), priv);
	on_theme_changed(settings, nullptr, priv);

	GMenuModel *app_menu = create_application_menu();
	g_action_map_add_action_entries(G_ACTION_MAP(application),
	                                app_entries, G_N_ELEMENTS(app_entries),
	                                priv);
	gtk_application_set_accels_for_action(GTK_APPLICATION(application), "app.quit", quit_accels);
	gtk_application_set_app_menu(GTK_APPLICATION(application), app_menu);
}

static void
reader_application_open(GApplication *application,
                        GFile **files,
                        gint n_files,
                        const gchar *hint)
{
	ReaderApplicationPrivate *priv = READER_APPLICATION_PRIVATE(application);
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
	{
		window = reader_window_new(GTK_APPLICATION(application), priv->settings, priv->tts, filename);
		gtk_widget_show_all(GTK_WIDGET(window));
	}
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
	G_OBJECT_CLASS(klass)->finalize = reader_application_finalize;
	G_APPLICATION_CLASS(klass)->startup = reader_application_startup;
	G_APPLICATION_CLASS(klass)->activate = reader_application_activate;
	G_APPLICATION_CLASS(klass)->open = reader_application_open;
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

void reader_application_set_active_document(ReaderApplication *application,
                                            CainteoirDocument *doc)
{
	ReaderApplicationPrivate *priv = READER_APPLICATION_PRIVATE(application);
	if (priv->doc) g_object_unref(priv->doc);
	priv->doc = CAINTEOIR_DOCUMENT(g_object_ref(G_OBJECT(doc)));
}
