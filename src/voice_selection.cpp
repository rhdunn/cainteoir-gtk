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

#include <sigc++/signal.h>

#include "voice_selection.hpp"

enum VoiceListColumns
{
	VLC_VOICE,
	VLC_ENGINE,
	VLC_LANGUAGE,
	VLC_REGION,
	VLC_GENDER,
	VLC_FREQUENCY,
	VLC_CHANNELS,
	VLC_URI,
	VLC_COUNT // number of columns
};

const char * columns[VLC_COUNT] = {
	i18n("Voice"),
	i18n("Engine"),
	i18n("Language"),
	i18n("Region"),
	i18n("Gender"),
	i18n("Frequency (Hz)"),
	i18n("Channels"),
};

static const char *bold(const char *text)
{
	static char boldtext[512];
	snprintf(boldtext, sizeof(boldtext), "<b>%s</b>", text);
	return boldtext;
}

static void on_voice_list_column_clicked(GtkTreeViewColumn *column, void *data)
{
	application_settings &settings = *(application_settings *)data;

	settings("voicelist.sort.column") = gtk_tree_view_column_get_sort_column_id(column);
	settings("voicelist.sort.order")  = (int)gtk_tree_view_column_get_sort_order(column);
}

struct voice_list_selection
{
	const rdf::uri *voice;
	GtkTreeSelection *selection;
};

static gboolean select_voice_list_item_by_voice(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, void *data)
{
	voice_list_selection *vls = (voice_list_selection *)data;

	gboolean ret = FALSE;
	gchar *voice = NULL;
	gtk_tree_model_get(model, iter, VLC_URI, &voice, -1);
	if (vls->voice->str() == voice)
	{
		gtk_tree_selection_select_iter(vls->selection, iter);
		ret = TRUE;
	}
	g_free(voice);
	return ret;
}

static void on_apply_button_clicked(GtkButton *button, void *data)
{
	((VoiceSelectionView *)data)->apply();
}

static void on_filter_by_doc_lang_active(GtkSwitch *button, GParamSpec *arg, void *data)
{
	bool active = gtk_switch_get_active(button);
	((VoiceList *)data)->filter_by_doc_lang(gtk_switch_get_active(button));
}

VoiceList::VoiceList(application_settings &aSettings, rdf::graph &aMetadata, cainteoir::languages &aLanguages)
	: settings(aSettings)
	, mMetadata(aMetadata)
	, languages(aLanguages)
	, doc_lang(cainteoir::language::make_lang("en"))
	, filter_by_doc_language(true)
{
	int         sort_column = settings("voicelist.sort.column", 0).as<int>();
	GtkSortType sort_order  = (GtkSortType)settings("voicelist.sort.order",  GTK_SORT_ASCENDING).as<int>();

	store = gtk_tree_store_new(VLC_COUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), sort_column, sort_order);

	GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	for (int i = 0; i < VLC_COUNT; ++i)
	{
		if (i == VLC_URI) continue;

		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
		GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(gettext(columns[i]), renderer, "text", i, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
		gtk_tree_view_column_set_sort_column_id(column, i);
		g_signal_connect(column, "clicked", G_CALLBACK(on_voice_list_column_clicked), &settings);
		if (i == sort_column)
		{
			gtk_tree_view_column_set_sort_indicator(column, TRUE);
			gtk_tree_view_column_set_sort_order(column, sort_order);
		}
	}

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
	refresh();

	layout = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(layout), tree);
}

void VoiceList::set_voice(const rdf::uri &voice)
{
	voice_list_selection vls = { &voice, selection };
	gtk_tree_model_foreach(GTK_TREE_MODEL(store), select_voice_list_item_by_voice, &vls);
}

const rdf::uri VoiceList::get_voice() const
{
	rdf::uri ref;
	GtkTreeIter row;
	if (gtk_tree_selection_get_selected(selection, NULL, &row))
	{
		gchar *voice = NULL;
		gtk_tree_model_get(GTK_TREE_MODEL(store), &row, VLC_URI, &voice, -1);
		ref = mMetadata.href(voice);
		g_free(voice);
	}
	return ref;
}

void VoiceList::set_language(const std::string &language)
{
	doc_lang = cainteoir::language::make_lang(language);
	refresh();
}

void VoiceList::filter_by_doc_lang(bool filter)
{
	filter_by_doc_language = filter;
	refresh();
}

void VoiceList::refresh()
{
	rdf::uri voice = get_voice();
	if (!voice.empty())
		selected_voice = voice;

	gtk_tree_store_clear(store);
	rql::results voicelist = rql::select(mMetadata,
		rql::both(rql::matches(rql::predicate, rdf::rdf("type")),
		          rql::matches(rql::object,    rdf::tts("Voice"))));

	foreach_iter(voice, voicelist)
	{
		rql::results statements = rql::select(mMetadata, rql::matches(rql::subject, rql::subject(*voice)));
		if (filter_by_doc_language)
		{
			std::string lang = rql::select_value<std::string>(statements,
				rql::matches(rql::predicate, rdf::dc("language")));

			if (cainteoir::language::make_lang(lang) == doc_lang)
				add_voice(mMetadata, statements, languages);
		}
		else
			add_voice(mMetadata, statements, languages);
	}

	set_voice(selected_voice);
}

void VoiceList::add_voice(rdf::graph &aMetadata, rql::results &voice, cainteoir::languages &languages)
{
	GtkTreeIter row;
	gtk_tree_store_append(store, &row, NULL);

	foreach_iter(statement, voice)
	{
		if (rql::predicate(*statement) == rdf::tts("name"))
			gtk_tree_store_set(store, &row,
				VLC_URI,   rql::subject(*statement).str().c_str(),
				VLC_VOICE, rql::value(*statement).c_str(),
				-1);
		else if (rql::predicate(*statement) == rdf::tts("voiceOf"))
		{
			std::string engine = rql::select_value<std::string>(aMetadata,
				rql::both(rql::matches(rql::subject,   rql::object(*statement)),
				          rql::matches(rql::predicate, rdf::tts("name"))));
			gtk_tree_store_set(store, &row, VLC_ENGINE, engine.c_str(), -1);
		}
		else if (rql::predicate(*statement) == rdf::dc("language"))
		{
			cainteoir::language::tag lang = cainteoir::language::make_lang(rql::value(*statement));
			gtk_tree_store_set(store, &row, VLC_LANGUAGE, languages.language(lang), -1);
			gtk_tree_store_set(store, &row, VLC_REGION, languages.region(lang), -1);
		}
		else if (rql::predicate(*statement) == rdf::tts("gender"))
		{
			if (rql::object(*statement) == rdf::tts("male"))
				gtk_tree_store_set(store, &row, VLC_GENDER, i18n("male"), -1);
			else if (rql::object(*statement) == rdf::tts("female"))
				gtk_tree_store_set(store, &row, VLC_GENDER, i18n("female"), -1);
		}
		else if (rql::predicate(*statement) == rdf::tts("frequency"))
			gtk_tree_store_set(store, &row, VLC_FREQUENCY, rql::value(*statement).c_str(), -1);
		else if (rql::predicate(*statement) == rdf::tts("channels"))
			gtk_tree_store_set(store, &row, VLC_CHANNELS, rql::value(*statement) == "1" ? i18n("mono") : i18n("stereo"), -1);
	}
}

VoiceSelectionView::VoiceSelectionView(application_settings &settings, tts::engines &aEngines, rdf::graph &aMetadata, cainteoir::languages &languages)
	: mEngines(&aEngines)
	, voices(settings, aMetadata, languages)
{
	layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_set_border_width(GTK_CONTAINER(layout), 6);

	GtkWidget *voices_header = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(voices_header), 0, 0);
	gtk_label_set_markup(GTK_LABEL(voices_header), bold(i18n("Voices")));

	GtkWidget *header = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(header), 0, 0);
	gtk_label_set_markup(GTK_LABEL(header), bold(i18n("Settings")));

	GtkWidget *filter_by_doc_lang_label = gtk_label_new(i18n("Filter voices by the document's language."));
	gtk_misc_set_alignment(GTK_MISC(filter_by_doc_lang_label), 1, 0.5);

	GtkWidget *filter_by_doc_lang = gtk_switch_new();
	gtk_switch_set_active(GTK_SWITCH(filter_by_doc_lang), TRUE);

	parameterView = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(parameterView), 4);

	create_entry(tts::parameter::rate, 0);
	create_entry(tts::parameter::volume, 1);
	create_entry(tts::parameter::pitch, 2);
	create_entry(tts::parameter::pitch_range, 3);

	GtkWidget *buttons = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(buttons), GTK_BUTTONBOX_START);

	GtkWidget *bottom = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(bottom), buttons, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(bottom), filter_by_doc_lang_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(bottom), filter_by_doc_lang, FALSE, FALSE, 0);

	GtkWidget *apply = gtk_button_new_with_mnemonic(i18n("_Apply"));
	gtk_box_pack_start(GTK_BOX(buttons), apply, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(gobj()), header, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gobj()), parameterView, FALSE, FALSE, 6);
	gtk_box_pack_start(GTK_BOX(gobj()), voices_header, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gobj()), voices, TRUE, TRUE, 6);
	gtk_box_pack_start(GTK_BOX(gobj()), bottom, FALSE, FALSE, 6);

	g_signal_connect(apply, "clicked", G_CALLBACK(on_apply_button_clicked), this);
	g_signal_connect(filter_by_doc_lang, "notify::active", G_CALLBACK(on_filter_by_doc_lang_active), &voices);
}

void VoiceSelectionView::show(const rdf::uri &voice)
{
	foreach_iter (item, parameters)
	{
		std::shared_ptr<tts::parameter> parameter = mEngines->parameter(item->type);

		gtk_range_set_range(GTK_RANGE(item->param), parameter->minimum(), parameter->maximum());
		gtk_range_set_value(GTK_RANGE(item->param), parameter->value());

		gtk_label_set_markup(GTK_LABEL(item->label), parameter->name());
		gtk_label_set_markup(GTK_LABEL(item->units), parameter->units());
	}

	voices.set_voice(voice);

	gtk_widget_show(layout);
}

void VoiceSelectionView::apply()
{
	foreach_iter (item, parameters)
	{
		mEngines->parameter(item->type)->set_value(gtk_range_get_value(GTK_RANGE(item->param)));
	}

	on_voice_change.emit(voices.get_voice());
}

void VoiceSelectionView::create_entry(tts::parameter::type aParameter, int row)
{
	VoiceParameter item;
	item.type = aParameter;

	item.param = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, NULL);
	gtk_range_set_increments(GTK_RANGE(item.param), 1.0, 5.0);
	gtk_scale_set_value_pos(GTK_SCALE(item.param), GTK_POS_RIGHT);
	gtk_scale_set_digits(GTK_SCALE(item.param), 0);
	gtk_widget_set_hexpand(item.param, TRUE);

	item.label = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(item.label), 1, 0.5);

	item.units = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(item.units), 0, 0.5);

	gtk_grid_attach(GTK_GRID(parameterView), item.label, 0, row, 1, 1);
	gtk_grid_attach(GTK_GRID(parameterView), item.param, 1, row, 1, 1);
	gtk_grid_attach(GTK_GRID(parameterView), item.units, 2, row, 1, 1);

	parameters.push_back(item);
}
