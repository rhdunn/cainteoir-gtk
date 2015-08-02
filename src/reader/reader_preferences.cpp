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
#include "reader_settings_view.h"

#include <cainteoir-gtk/cainteoir_settings.h>
#include <cainteoir-gtk/cainteoir_speech_synthesizers.h>
#include <cainteoir-gtk/cainteoir_speech_voice_view.h>

#include "extensions/glib.h"

struct ReaderPreferencesPrivate
{
	GtkWidget *tabs;

	CainteoirSettings *settings;
	CainteoirSpeechSynthesizers *tts;

	ReaderPreferencesPrivate()
		: settings(nullptr)
		, tts(nullptr)
	{
	}

	~ReaderPreferencesPrivate()
	{
		g_object_unref(G_OBJECT(settings));
		g_object_unref(G_OBJECT(tts));
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

static void
on_voice_changed(CainteoirSpeechVoiceView *view, const gchar *voice, ReaderPreferencesPrivate *priv)
{
	if (cainteoir_speech_synthesizers_set_voice(priv->tts, voice))
	{
		cainteoir_settings_set_string(priv->settings, "voice", "name", voice);
		cainteoir_settings_save(priv->settings);
	}
}

GtkWidget *
reader_preferences_new(ReaderWindow *window,
                       CainteoirSettings *settings,
                       CainteoirSpeechSynthesizers *synthesizers,
                       CainteoirDocument *doc)
{
	ReaderPreferences *prefs = READER_PREFERENCES(g_object_new(READER_TYPE_PREFERENCES,
		"transient-for", window,
		"use-header-bar", TRUE,
		nullptr));
	ReaderPreferencesPrivate *priv = READER_PREFERENCES_PRIVATE(prefs);
	priv->settings = CAINTEOIR_SETTINGS(g_object_ref(G_OBJECT(settings)));
	priv->tts = CAINTEOIR_SPEECH_SYNTHESIZERS(g_object_ref(G_OBJECT(synthesizers)));

	gtk_window_set_default_size(GTK_WINDOW(prefs), 500, 200);
	gtk_window_set_title(GTK_WINDOW(prefs), i18n("Preferences"));
	gtk_window_set_modal(GTK_WINDOW(prefs), FALSE);

	GtkWidget *layout = gtk_dialog_get_content_area(GTK_DIALOG(prefs));

	priv->tabs = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(layout), priv->tabs, TRUE, TRUE, 0);

	GtkWidget *voices  = cainteoir_speech_voice_view_new(synthesizers);
	g_signal_connect(voices, "voice-changed", G_CALLBACK(on_voice_changed), priv);

	GtkWidget *general = reader_settings_view_new(priv->settings, synthesizers, CAINTEOIR_SPEECH_VOICE_VIEW(voices), window);

	GtkWidget *scroll = gtk_scrolled_window_new(nullptr, nullptr);
	gtk_container_add(GTK_CONTAINER(scroll), voices);

	gtk_notebook_append_page(GTK_NOTEBOOK(priv->tabs), general, gtk_label_new(i18n("General")));
	gtk_notebook_append_page(GTK_NOTEBOOK(priv->tabs), scroll,  gtk_label_new(i18n("Voices")));

	gtk_widget_show_all(GTK_WIDGET(prefs));

	cainteoir_speech_voice_view_set_filter_language_from_document(CAINTEOIR_SPEECH_VOICE_VIEW(voices), doc);

	gchar *voice = cainteoir_settings_get_string(priv->settings, "voice", "name", nullptr);
	if (!voice)
		voice = cainteoir_speech_synthesizers_get_voice(priv->tts);
	cainteoir_speech_voice_view_set_voice(CAINTEOIR_SPEECH_VOICE_VIEW(voices), voice);
	g_free(voice);

	return GTK_WIDGET(prefs);
}
