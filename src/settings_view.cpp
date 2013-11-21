/* Voice Selection View
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

#include "settings_view.hpp"

struct setting_data_t
{
	tts::parameter::type type;
	const char *key;
	const char *label;
	const char *parameter;
	const char *units;
};

static const setting_data_t setting_entries[] = {
	{ tts::parameter::rate, "voice.rate",
	  "rate-label", "rate-parameter", "rate-units" },
	{ tts::parameter::volume, "voice.volume",
	  "volume-label", "volume-parameter", "volume-units" },
	{ tts::parameter::pitch, "voice.pitch",
	  "pitch-label", "pitch-parameter", "pitch-units" },
	{ tts::parameter::pitch_range, "voice.pitch-range",
	  "pitchrange-label", "pitchrange-parameter", "pitchrange-units" },
};

static void on_apply_button_clicked(GtkButton *button, void *data)
{
	((SettingsView *)data)->apply();
}

SettingsView::SettingsView(application_settings &aSettings, tts::engines &aEngines, GtkBuilder *ui)
	: mEngines(&aEngines)
	, settings(aSettings)
{
	layout = GTK_WIDGET(gtk_builder_get_object(ui, "settings-page"));

	for (auto &setting : setting_entries)
	{
		VoiceParameter item;
		item.type = setting.type;
		item.id = setting.key;
		item.label = GTK_WIDGET(gtk_builder_get_object(ui, setting.label));
		item.param = GTK_WIDGET(gtk_builder_get_object(ui, setting.parameter));
		item.units = GTK_WIDGET(gtk_builder_get_object(ui, setting.units));
		parameters.push_back(item);

		int value = mEngines->parameter(item.type)->value();
		mEngines->parameter(item.type)->set_value(settings(item.id, value).as<int>());
	}

	GtkWidget *apply = GTK_WIDGET(gtk_builder_get_object(ui, "apply-settings"));
	g_signal_connect(apply, "clicked", G_CALLBACK(on_apply_button_clicked), this);
}

void SettingsView::show()
{
	for (auto &item : parameters)
	{
		std::shared_ptr<tts::parameter> parameter = mEngines->parameter(item.type);

		gtk_range_set_range(GTK_RANGE(item.param), parameter->minimum(), parameter->maximum());
		gtk_range_set_value(GTK_RANGE(item.param), parameter->value());

		gtk_label_set_markup(GTK_LABEL(item.label), parameter->name());
		gtk_label_set_markup(GTK_LABEL(item.units), parameter->units());
	}

	gtk_widget_show(layout);
}

void SettingsView::apply()
{
	for (auto &item : parameters)
	{
		int value = gtk_range_get_value(GTK_RANGE(item.param));
		mEngines->parameter(item.type)->set_value(value);
		settings(item.id) = value;
	}
}
