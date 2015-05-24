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

struct SpeechParameterSetting
{
	GtkWidget *label;
	GtkWidget *value;
	GtkWidget *units;

	gulong signal;
	const gchar *key;

	CainteoirSettings *settings;
	CainteoirSpeechParameter *parameter;
};

static void
on_speech_parameter_changed(GtkRange *range, gpointer data)
{
	SpeechParameterSetting *setting = (SpeechParameterSetting *)data;

	cainteoir_settings_set_integer(setting->settings, "voice", setting->key,
	                               gtk_range_get_value(GTK_RANGE(setting->value)));
	cainteoir_settings_save(setting->settings);
}

static void
speech_parameter_setting_init(SpeechParameterSetting *setting, GtkWidget *grid, gint row, CainteoirSettings *settings, const gchar *key)
{
	setting->parameter = nullptr;
	setting->settings = settings;
	setting->signal = (gulong)-1;
	setting->key = key;

	setting->label = gtk_label_new(nullptr);
	gtk_widget_set_halign(setting->label, GTK_ALIGN_END);
	gtk_grid_attach(GTK_GRID(grid), setting->label, 0, row, 1, 1);

	setting->value = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, nullptr);
	gtk_widget_set_hexpand(setting->value, TRUE);
	gtk_range_set_round_digits(GTK_RANGE(setting->value), 0);
	gtk_scale_set_digits(GTK_SCALE(setting->value), 0);
	gtk_scale_set_value_pos(GTK_SCALE(setting->value), GTK_POS_RIGHT);
	gtk_grid_attach(GTK_GRID(grid), setting->value, 1, row, 1, 1);

	setting->units = gtk_label_new(nullptr);
	gtk_widget_set_halign(setting->units, GTK_ALIGN_START);
	gtk_grid_attach(GTK_GRID(grid), setting->units, 2, row, 1, 1);
}

static void
speech_parameter_setting_update(SpeechParameterSetting *setting,
                                CainteoirSpeechSynthesizers *synthesizers,
                                CainteoirSpeechParameterType parameter)
{
	if (setting->parameter) g_object_unref(G_OBJECT(setting->parameter));
	setting->parameter = cainteoir_speech_synthesizers_get_parameter(synthesizers, parameter);

	if (setting->signal != (gulong)-1) g_signal_handler_disconnect(setting->value, setting->signal);

	if (setting->parameter)
	{
		gtk_widget_show(setting->label);
		gtk_widget_show(setting->value);
		gtk_widget_show(setting->units);

		gtk_label_set_label(GTK_LABEL(setting->label), cainteoir_speech_parameter_get_name(setting->parameter));

		gtk_range_set_range(GTK_RANGE(setting->value),
		                              cainteoir_speech_parameter_get_minimum(setting->parameter),
		                              cainteoir_speech_parameter_get_maximum(setting->parameter));

		gint value = cainteoir_settings_get_integer(setting->settings, "voice", setting->key,
		                                            cainteoir_speech_parameter_get_default(setting->parameter));
		gtk_range_set_value(GTK_RANGE(setting->value), value);

		gtk_label_set_label(GTK_LABEL(setting->units), cainteoir_speech_parameter_get_units(setting->parameter));

		setting->signal = g_signal_connect(setting->value, "value-changed", G_CALLBACK(on_speech_parameter_changed), setting);
	}
	else
	{
		gtk_widget_hide(setting->label);
		gtk_widget_hide(setting->value);
		gtk_widget_hide(setting->units);
	}
}

struct _ReaderSettingsViewPrivate
{
	CainteoirSettings *settings;
	CainteoirSpeechSynthesizers *tts;

	SpeechParameterSetting rate;
	SpeechParameterSetting volume;
	SpeechParameterSetting pitch;
	SpeechParameterSetting pitch_range;
	SpeechParameterSetting word_gap;
};

G_DEFINE_TYPE_WITH_PRIVATE(ReaderSettingsView, reader_settings_view, GTK_TYPE_BIN)

static void
reader_settings_view_finalize(GObject *object)
{
	ReaderSettingsView *view = READER_SETTINGS_VIEW(object);
	if (view->priv->rate.parameter)        g_object_unref(G_OBJECT(view->priv->rate.parameter));
	if (view->priv->volume.parameter)      g_object_unref(G_OBJECT(view->priv->volume.parameter));
	if (view->priv->pitch.parameter)       g_object_unref(G_OBJECT(view->priv->pitch.parameter));
	if (view->priv->pitch_range.parameter) g_object_unref(G_OBJECT(view->priv->pitch_range.parameter));
	if (view->priv->word_gap.parameter)    g_object_unref(G_OBJECT(view->priv->word_gap.parameter));
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

	GtkWidget *parameter_grid = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(parameter_grid), 5);
	gtk_grid_set_column_spacing(GTK_GRID(parameter_grid), 5);
	gtk_box_pack_start(GTK_BOX(layout), parameter_grid, FALSE, FALSE, 5);

	speech_parameter_setting_init(&view->priv->rate, parameter_grid, 0, view->priv->settings, "rate");
	speech_parameter_setting_init(&view->priv->volume, parameter_grid, 1, view->priv->settings, "volume");
	speech_parameter_setting_init(&view->priv->pitch, parameter_grid, 2, view->priv->settings, "pitch");
	speech_parameter_setting_init(&view->priv->pitch_range, parameter_grid, 3, view->priv->settings, "pitch-range");
	speech_parameter_setting_init(&view->priv->word_gap, parameter_grid, 4, view->priv->settings, "word-gap");

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
	reader_settings_view_update_speech_parameters(view);

	return GTK_WIDGET(view);
}

void
reader_settings_view_update_speech_parameters(ReaderSettingsView *view)
{
	speech_parameter_setting_update(&view->priv->rate, view->priv->tts, CAINTEOIR_SPEECH_RATE);
	speech_parameter_setting_update(&view->priv->volume, view->priv->tts, CAINTEOIR_SPEECH_VOLUME);
	speech_parameter_setting_update(&view->priv->pitch, view->priv->tts, CAINTEOIR_SPEECH_PITCH);
	speech_parameter_setting_update(&view->priv->pitch_range, view->priv->tts, CAINTEOIR_SPEECH_PITCH_RANGE);
	speech_parameter_setting_update(&view->priv->word_gap, view->priv->tts, CAINTEOIR_SPEECH_WORD_GAP);
}
