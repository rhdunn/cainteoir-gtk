/* The Cainteoir Reader settings view.
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

#include "reader_settings_view.h"

struct _ReaderSettingsViewPrivate
{
	CainteoirSettings *settings;
	CainteoirSpeechSynthesizers *tts;
};

G_DEFINE_TYPE_WITH_PRIVATE(ReaderSettingsView, reader_settings_view, GTK_TYPE_BIN)

static void
reader_settings_view_finalize(GObject *object)
{
	ReaderSettingsView *view = READER_SETTINGS_VIEW(object);
	g_object_unref(G_OBJECT(view->priv->settings));
	g_object_unref(G_OBJECT(view->priv->tts));

	G_OBJECT_CLASS(reader_settings_view_parent_class)->finalize(object);
}

static void
reader_settings_view_class_init(ReaderSettingsViewClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = reader_settings_view_finalize;
}

static void
reader_settings_view_init(ReaderSettingsView *view)
{
	view->priv = (ReaderSettingsViewPrivate *)reader_settings_view_get_instance_private(view);
}

static void
update_narration_mode(ReaderSettingsView *view)
{
	CainteoirNarration narration = CAINTEOIR_NARRATION_TTS_ONLY;
	if (cainteoir_settings_get_boolean(view->priv->settings, "narration", "enabled", FALSE))
	{
		if (cainteoir_settings_get_boolean(view->priv->settings, "narration", "tts-fallback", FALSE))
			narration = CAINTEOIR_NARRATION_TTS_AND_MEDIA_OVERLAYS;
		else
			narration = CAINTEOIR_NARRATION_MEDIA_OVERLAYS_ONLY;
	}
	cainteoir_speech_synthesizers_set_narration(view->priv->tts, narration);
}

static void
on_narration_active(GtkWidget *narration, GdkEvent *event, gpointer data)
{
	ReaderSettingsView *view = (ReaderSettingsView *)data;

	cainteoir_settings_set_boolean(view->priv->settings, "narration", "enabled",
	                               gtk_switch_get_active(GTK_SWITCH(narration)));
	cainteoir_settings_save(view->priv->settings);

	update_narration_mode(view);
}

static void
on_tts_fallback_active(GtkWidget *tts_fallback, GdkEvent *event, gpointer data)
{
	ReaderSettingsView *view = (ReaderSettingsView *)data;

	cainteoir_settings_set_boolean(view->priv->settings, "narration", "tts-fallback",
	                               gtk_switch_get_active(GTK_SWITCH(tts_fallback)));
	cainteoir_settings_save(view->priv->settings);

	update_narration_mode(view);
}

GtkWidget *
reader_settings_view_new(CainteoirSettings *settings,
                         CainteoirSpeechSynthesizers *synthesizers)
{
	ReaderSettingsView *view = READER_SETTINGS_VIEW(g_object_new(READER_TYPE_SETTINGS_VIEW, nullptr));
	view->priv->settings = CAINTEOIR_SETTINGS(g_object_ref(G_OBJECT(settings)));
	view->priv->tts = CAINTEOIR_SPEECH_SYNTHESIZERS(g_object_ref(G_OBJECT(synthesizers)));

#if GTK_CHECK_VERSION(3, 12, 0)
	gtk_widget_set_margin_start(GTK_WIDGET(view), 5);
	gtk_widget_set_margin_end(GTK_WIDGET(view), 5);
#else
	gtk_widget_set_margin_left(GTK_WIDGET(view), 5);
	gtk_widget_set_margin_right(GTK_WIDGET(view), 5);
#endif

	GtkWidget *layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_add(GTK_CONTAINER(view), layout);

	GtkWidget *settings_grid = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(settings_grid), 5);
	gtk_grid_set_column_spacing(GTK_GRID(settings_grid), 5);
	gtk_box_pack_start(GTK_BOX(layout), settings_grid, TRUE, TRUE, 5);

	GtkWidget *narration_label = gtk_label_new(i18n("Narration"));
	gtk_widget_set_hexpand(narration_label, TRUE);
	gtk_widget_set_halign(narration_label, GTK_ALIGN_START);
	gtk_grid_attach(GTK_GRID(settings_grid), narration_label, 0, 0, 1, 1);

	GtkWidget *narration = gtk_switch_new();
	gtk_grid_attach(GTK_GRID(settings_grid), narration, 1, 0, 1, 1);
	gtk_switch_set_active(GTK_SWITCH(narration),
	                      cainteoir_settings_get_boolean(view->priv->settings, "narration", "enabled", FALSE));
	g_signal_connect(narration, "notify::active", G_CALLBACK(on_narration_active), view);

	GtkWidget *tts_fallback_label = gtk_label_new(i18n("Text-to-Speech fallback"));
	gtk_widget_set_hexpand(tts_fallback_label, TRUE);
	gtk_widget_set_halign(tts_fallback_label, GTK_ALIGN_START);
	gtk_grid_attach(GTK_GRID(settings_grid), tts_fallback_label, 0, 1, 1, 1);

	GtkWidget *tts_fallback = gtk_switch_new();
	gtk_grid_attach(GTK_GRID(settings_grid), tts_fallback, 1, 1, 1, 1);
	gtk_switch_set_active(GTK_SWITCH(tts_fallback),
	                      cainteoir_settings_get_boolean(view->priv->settings, "narration", "tts-fallback", FALSE));
	g_signal_connect(tts_fallback, "notify::active", G_CALLBACK(on_tts_fallback_active), view);

	update_narration_mode(view);

	return GTK_WIDGET(view);
}
