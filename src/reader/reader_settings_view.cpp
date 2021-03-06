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
#include <string.h>

#include "reader_settings_view.h"
#include "reader_window.h"

#include <cainteoir-gtk/cainteoir_speech_synthesizers.h>
#include <cainteoir-gtk/cainteoir_speech_voice_view.h>
#include <cainteoir-gtk/cainteoir_speech_parameter.h>
#include <cainteoir-gtk/cainteoir_settings.h>

#include "extensions/glib.h"

enum HighlightAnchorColumns
{
	HIGHLIGHT_ANCHOR_LABEL,
	HIGHLIGHT_ANCHOR_ID,
	HIGHLIGHT_ANCHOR_ALIGN,
	HIGHLIGHT_ANCHOR_COUNT,
};

struct SpeechParameterSetting
{
	GtkWidget *label;
	GtkWidget *value;
	GtkWidget *units;
	GtkWidget *reset;

	gulong signal;
	const gchar *key;

	CainteoirSettings *settings;
	CainteoirSpeechParameter *parameter;

	~SpeechParameterSetting()
	{
		if (parameter) g_object_unref(G_OBJECT(parameter));
	}
};

static void
on_speech_parameter_changed(GtkRange *range, SpeechParameterSetting *setting)
{
	guint value = gtk_range_get_value(GTK_RANGE(setting->value));

	cainteoir_speech_parameter_set_value(setting->parameter, value);

	cainteoir_settings_set_integer(setting->settings, "voice", setting->key, value);
	cainteoir_settings_save(setting->settings);
}

static void
on_speech_parameter_reset(GtkWidget *range, SpeechParameterSetting *setting)
{
	gtk_range_set_value(GTK_RANGE(setting->value), cainteoir_speech_parameter_get_default(setting->parameter));
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

	setting->reset = gtk_button_new_with_label(i18n("Reset"));
	gtk_grid_attach(GTK_GRID(grid), setting->reset, 3, row, 1, 1);
	g_signal_connect(setting->reset, "clicked", G_CALLBACK(on_speech_parameter_reset), setting);
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

		cainteoir_speech_parameter_set_value(setting->parameter, value);

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

struct ReaderSettingsViewPrivate
{
	CainteoirSettings *settings;
	CainteoirSpeechSynthesizers *tts;
	CainteoirSpeechVoiceView *voice_view;
	ReaderWindow *reader;

	SpeechParameterSetting rate;
	SpeechParameterSetting volume;
	SpeechParameterSetting pitch;
	SpeechParameterSetting pitch_range;
	SpeechParameterSetting word_gap;

	~ReaderSettingsViewPrivate()
	{
		g_object_unref(G_OBJECT(settings));
		g_object_unref(G_OBJECT(tts));
		g_object_unref(G_OBJECT(voice_view));
	}
};

G_DEFINE_TYPE_WITH_PRIVATE(ReaderSettingsView, reader_settings_view, GTK_TYPE_BIN)

#define READER_SETTINGS_VIEW_PRIVATE(object) \
	((ReaderSettingsViewPrivate *)reader_settings_view_get_instance_private(READER_SETTINGS_VIEW(object)))

GXT_DEFINE_TYPE_CONSTRUCTION(ReaderSettingsView, reader_settings_view, READER_SETTINGS_VIEW)

static void
reader_settings_view_class_init(ReaderSettingsViewClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = reader_settings_view_finalize;
}

static void
update_narration_mode(ReaderSettingsViewPrivate *priv)
{
	CainteoirNarration narration = CAINTEOIR_NARRATION_TTS_ONLY;
	if (cainteoir_settings_get_boolean(priv->settings, "narration", "enabled", FALSE))
	{
		if (cainteoir_settings_get_boolean(priv->settings, "narration", "tts-fallback", FALSE))
			narration = CAINTEOIR_NARRATION_TTS_AND_MEDIA_OVERLAYS;
		else
			narration = CAINTEOIR_NARRATION_MEDIA_OVERLAYS_ONLY;
	}
	cainteoir_speech_synthesizers_set_narration(priv->tts, narration);
}

static void
on_narration_active(GtkWidget *narration, GdkEvent *event, ReaderSettingsViewPrivate *priv)
{
	cainteoir_settings_set_boolean(priv->settings, "narration", "enabled",
	                               gtk_switch_get_active(GTK_SWITCH(narration)));
	cainteoir_settings_save(priv->settings);

	update_narration_mode(priv);
}

static void
on_tts_fallback_active(GtkWidget *tts_fallback, GdkEvent *event, ReaderSettingsViewPrivate *priv)
{
	cainteoir_settings_set_boolean(priv->settings, "narration", "tts-fallback",
	                               gtk_switch_get_active(GTK_SWITCH(tts_fallback)));
	cainteoir_settings_save(priv->settings);

	update_narration_mode(priv);
}

static void
on_highlight_active(GtkWidget *highlight, GdkEvent *event, ReaderSettingsViewPrivate *priv)
{
	gboolean active = gtk_switch_get_active(GTK_SWITCH(highlight));

	cainteoir_speech_synthesizers_set_text_event_mode(priv->tts,
		active ? CAINTEOIR_TEXT_EVENT_WHILE_READING : CAINTEOIR_TEXT_EVENT_NONE);

	cainteoir_settings_set_string(priv->settings, "highlight", "mode", active ? "reading" : "none");
	cainteoir_settings_save(priv->settings);
}

static void
on_filter_voices_by_doclang_active(GtkWidget *filter_voices_by_doclang, GdkEvent *event, ReaderSettingsViewPrivate *priv)
{
	gboolean active = gtk_switch_get_active(GTK_SWITCH(filter_voices_by_doclang));

	cainteoir_speech_voice_view_set_filter(priv->voice_view,
	                                       active ? CAINTEOIR_VOICE_FILTER_BY_LANGUAGE : CAINTEOIR_VOICE_FILTER_ALL);

	cainteoir_settings_set_string(priv->settings, "voicelist", "filter", active ? "language" : "all");
	cainteoir_settings_save(priv->settings);
}

static void
on_highlight_anchor_changed(GtkWidget *widget, ReaderSettingsViewPrivate *priv)
{
	GtkTreeIter item;
	if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &item))
	{
		gchar *id = nullptr;
		GtkAlign align = GTK_ALIGN_FILL;

		GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
		gtk_tree_model_get(model, &item, HIGHLIGHT_ANCHOR_ID, &id, HIGHLIGHT_ANCHOR_ALIGN, &align, -1);

		cainteoir_settings_set_string(priv->settings, "highlight", "anchor", id);
		cainteoir_settings_save(priv->settings);

		reader_window_set_highlight_anchor(priv->reader, align);

		g_free(id);
	}
}

static GtkWidget *
create_highlight_anchor_combo()
{
	GtkTreeStore *store = gtk_tree_store_new(HIGHLIGHT_ANCHOR_COUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
	GtkTreeIter row;

	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
	                   HIGHLIGHT_ANCHOR_LABEL, i18n("None"),
	                   HIGHLIGHT_ANCHOR_ID,    "none",
	                   HIGHLIGHT_ANCHOR_ALIGN, GTK_ALIGN_FILL,
	                   -1);

	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
	                   HIGHLIGHT_ANCHOR_LABEL, i18n("Top"),
	                   HIGHLIGHT_ANCHOR_ID,    "top",
	                   HIGHLIGHT_ANCHOR_ALIGN, GTK_ALIGN_START,
	                   -1);

	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
	                   HIGHLIGHT_ANCHOR_LABEL, i18n("Middle"),
	                   HIGHLIGHT_ANCHOR_ID,    "middle",
	                   HIGHLIGHT_ANCHOR_ALIGN, GTK_ALIGN_CENTER,
	                   -1);

	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
	                   HIGHLIGHT_ANCHOR_LABEL, i18n("Bottom"),
	                   HIGHLIGHT_ANCHOR_ID,    "bottom",
	                   HIGHLIGHT_ANCHOR_ALIGN, GTK_ALIGN_END,
	                   -1);

	GtkWidget *combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_combo_box_set_id_column(GTK_COMBO_BOX(combo), HIGHLIGHT_ANCHOR_ID);

	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer, "text", HIGHLIGHT_ANCHOR_LABEL, nullptr);

	g_object_unref(G_OBJECT(store));
	return combo;
}

GtkWidget *
reader_settings_view_new(CainteoirSettings *settings,
                         CainteoirSpeechSynthesizers *synthesizers,
                         CainteoirSpeechVoiceView *voice_view,
                         ReaderWindow *reader)
{
	ReaderSettingsView *view = READER_SETTINGS_VIEW(g_object_new(READER_TYPE_SETTINGS_VIEW, nullptr));
	ReaderSettingsViewPrivate *priv = READER_SETTINGS_VIEW_PRIVATE(view);
	priv->settings = CAINTEOIR_SETTINGS(g_object_ref(G_OBJECT(settings)));
	priv->tts = CAINTEOIR_SPEECH_SYNTHESIZERS(g_object_ref(G_OBJECT(synthesizers)));
	priv->voice_view = CAINTEOIR_SPEECH_VOICE_VIEW(g_object_ref(G_OBJECT(voice_view)));
	priv->reader = reader;

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

	speech_parameter_setting_init(&priv->rate, parameter_grid, 0, priv->settings, "rate");
	speech_parameter_setting_init(&priv->volume, parameter_grid, 1, priv->settings, "volume");
	speech_parameter_setting_init(&priv->pitch, parameter_grid, 2, priv->settings, "pitch");
	speech_parameter_setting_init(&priv->pitch_range, parameter_grid, 3, priv->settings, "pitch-range");
	speech_parameter_setting_init(&priv->word_gap, parameter_grid, 4, priv->settings, "word-gap");

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
	                      cainteoir_settings_get_boolean(priv->settings, "narration", "enabled", FALSE));
	g_signal_connect(narration, "notify::active", G_CALLBACK(on_narration_active), priv);

	GtkWidget *tts_fallback_label = gtk_label_new(i18n("Text-to-Speech fallback"));
	gtk_widget_set_hexpand(tts_fallback_label, TRUE);
	gtk_widget_set_halign(tts_fallback_label, GTK_ALIGN_START);
	gtk_grid_attach(GTK_GRID(settings_grid), tts_fallback_label, 0, 1, 1, 1);

	GtkWidget *tts_fallback = gtk_switch_new();
	gtk_grid_attach(GTK_GRID(settings_grid), tts_fallback, 1, 1, 1, 1);
	gtk_switch_set_active(GTK_SWITCH(tts_fallback),
	                      cainteoir_settings_get_boolean(priv->settings, "narration", "tts-fallback", FALSE));
	g_signal_connect(tts_fallback, "notify::active", G_CALLBACK(on_tts_fallback_active), priv);

	GtkWidget *highlight_label = gtk_label_new(i18n("Highlight text while reading"));
	gtk_widget_set_hexpand(highlight_label, TRUE);
	gtk_widget_set_halign(highlight_label, GTK_ALIGN_START);
	gtk_grid_attach(GTK_GRID(settings_grid), highlight_label, 0, 2, 1, 1);

	GtkWidget *highlight = gtk_switch_new();
	gtk_grid_attach(GTK_GRID(settings_grid), highlight, 1, 2, 1, 1);
	g_signal_connect(highlight, "notify::active", G_CALLBACK(on_highlight_active), priv);

	GtkWidget *highlight_anchor_label = gtk_label_new(i18n("Scroll highlighted text to"));
	gtk_widget_set_hexpand(highlight_anchor_label, TRUE);
	gtk_widget_set_halign(highlight_anchor_label, GTK_ALIGN_START);
	gtk_grid_attach(GTK_GRID(settings_grid), highlight_anchor_label, 0, 3, 1, 1);

	GtkWidget *highlight_anchor = create_highlight_anchor_combo();
	gtk_grid_attach(GTK_GRID(settings_grid), highlight_anchor, 1, 3, 1, 1);
	g_signal_connect(highlight_anchor, "changed", G_CALLBACK(on_highlight_anchor_changed), priv);

	GtkWidget *filter_voices_by_doclang_label = gtk_label_new(i18n("Filter voices by document language"));
	gtk_widget_set_hexpand(filter_voices_by_doclang_label, TRUE);
	gtk_widget_set_halign(filter_voices_by_doclang_label, GTK_ALIGN_START);
	gtk_grid_attach(GTK_GRID(settings_grid), filter_voices_by_doclang_label, 0, 4, 1, 1);

	GtkWidget *filter_voices_by_doclang = gtk_switch_new();
	gtk_grid_attach(GTK_GRID(settings_grid), filter_voices_by_doclang, 1, 4, 1, 1);
	g_signal_connect(filter_voices_by_doclang, "notify::active", G_CALLBACK(on_filter_voices_by_doclang_active), priv);

	gchar *filter = cainteoir_settings_get_string(priv->settings, "voicelist", "filter", "all");
	if (filter)
	{
		gtk_switch_set_active(GTK_SWITCH(filter_voices_by_doclang), !strcmp(filter, "language"));
		g_free(filter);
	}

	gchar *highlight_mode = cainteoir_settings_get_string(priv->settings, "highlight", "mode", "reading");
	if (highlight_mode)
	{
		bool reading = !strcmp(highlight_mode, "reading");
		cainteoir_speech_synthesizers_set_text_event_mode(priv->tts,
			reading ? CAINTEOIR_TEXT_EVENT_WHILE_READING : CAINTEOIR_TEXT_EVENT_NONE);
		gtk_switch_set_active(GTK_SWITCH(highlight), reading);
		g_free(highlight_mode);
	}

	gchar *anchor = cainteoir_settings_get_string(priv->settings, "highlight", "anchor", "none");
	gtk_combo_box_set_active_id(GTK_COMBO_BOX(highlight_anchor), anchor);
	g_free(anchor);

	update_narration_mode(priv);
	reader_settings_view_update_speech_parameters(view);

	return GTK_WIDGET(view);
}

void
reader_settings_view_update_speech_parameters(ReaderSettingsView *view)
{
	ReaderSettingsViewPrivate *priv = READER_SETTINGS_VIEW_PRIVATE(view);
	speech_parameter_setting_update(&priv->rate, priv->tts, CAINTEOIR_SPEECH_RATE);
	speech_parameter_setting_update(&priv->volume, priv->tts, CAINTEOIR_SPEECH_VOLUME);
	speech_parameter_setting_update(&priv->pitch, priv->tts, CAINTEOIR_SPEECH_PITCH);
	speech_parameter_setting_update(&priv->pitch_range, priv->tts, CAINTEOIR_SPEECH_PITCH_RANGE);
	speech_parameter_setting_update(&priv->word_gap, priv->tts, CAINTEOIR_SPEECH_WORD_GAP);
}
