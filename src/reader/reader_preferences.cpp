/* The Cainteoir Reader preferences dialog.
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

#include "reader_preferences.h"

#include <cainteoir-gtk/cainteoir_settings.h>

#include "extensions/glib.h"

struct ReaderPreferencesPrivate
{
	CainteoirSettings *settings;

	ReaderPreferencesPrivate()
		: settings(nullptr)
	{
	}

	~ReaderPreferencesPrivate()
	{
		g_object_unref(G_OBJECT(settings));
	}
};

G_DEFINE_TYPE_WITH_PRIVATE(ReaderPreferences, reader_preferences, GTK_TYPE_DIALOG)

#define READER_PREFERENCES_PRIVATE(object) \
	((ReaderPreferencesPrivate *)reader_preferences_get_instance_private(READER_PREFERENCES(object)))

GXT_DEFINE_TYPE_CONSTRUCTION(ReaderPreferences, reader_preferences, READER_PREFERENCES)

static void
reader_preferences_class_init(ReaderPreferencesClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = reader_preferences_finalize;
}

GtkWidget *
reader_preferences_new(ReaderWindow *window,
                       CainteoirSettings *settings)
{
	ReaderPreferences *prefs = READER_PREFERENCES(g_object_new(READER_TYPE_PREFERENCES,
		"transient-for", window,
		"use-header-bar", TRUE,
		nullptr));
	ReaderPreferencesPrivate *priv = READER_PREFERENCES_PRIVATE(prefs);
	priv->settings = CAINTEOIR_SETTINGS(g_object_ref(G_OBJECT(settings)));

	gtk_window_set_default_size(GTK_WINDOW(prefs), 300, 200);
	gtk_window_set_title(GTK_WINDOW(prefs), i18n("Preferences"));
	gtk_window_set_modal(GTK_WINDOW(prefs), TRUE);

	return GTK_WIDGET(prefs);
}
