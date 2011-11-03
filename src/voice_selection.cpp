/* Voice Selection View
 *
 * Copyright (C) 2011 Reece H. Dunn
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

#include <config.h>
#include <gtkmm.h>
#include <cainteoir/platform.hpp>

#include "voice_selection.hpp"

enum VoiceListColumns
{
	VLC_VOICE,
	VLC_ENGINE,
	VLC_LANGUAGE,
	VLC_GENDER,
	VLC_AGE,
	VLC_FREQUENCY,
	VLC_CHANNELS,
	VLC_COUNT // number of columns
};

const char * columns[VLC_COUNT] = {
	_("Voice"),
	_("Engine"),
	_("Language"),
	_("Gender"),
	_("Age"),
	_("Frequency (Hz)"),
	_("Channels"),
};

VoiceList::VoiceList(rdf::graph &aMetadata, cainteoir::languages &languages)
{
	store = gtk_tree_store_new(VLC_COUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	tree  = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

	for (int i = 0; i < VLC_COUNT; ++i)
	{
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
		GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(columns[i], renderer, "text", i, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	}

	rql::results voicelist = rql::select(aMetadata,
		rql::both(rql::matches(rql::predicate, rdf::rdf("type")),
		          rql::matches(rql::object, rdf::tts("Voice"))));

	foreach_iter(voice, voicelist)
	{
		const rdf::uri *uri = rql::subject(*voice);
		if (uri)
		{
			rql::results statements = rql::select(aMetadata, rql::matches(rql::subject, *uri));
			add_voice(aMetadata, statements, languages);
		}
	}

	layout = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(layout), tree);

	// FIXME: The scrolled treeview takes up a minimal height, so set a default
	// height to make it useable. The height should be calculated dynamically
	// so the treeview fills the content area.
	gtk_widget_set_size_request(layout, -1, 258);
}

void VoiceList::add_voice(rdf::graph &aMetadata, rql::results &voice, cainteoir::languages &languages)
{
	GtkTreeIter row;
	gtk_tree_store_append(store, &row, NULL);

	foreach_iter(statement, voice)
	{
		if (rql::predicate(*statement) == rdf::tts("name"))
			gtk_tree_store_set(store, &row, VLC_VOICE, rql::value(*statement).c_str(), -1);
		else if (rql::predicate(*statement) == rdf::tts("voiceOf"))
		{
			std::string engine = rql::select_value<std::string>(aMetadata,
				rql::both(rql::matches(rql::subject,   rql::object(*statement)),
				          rql::matches(rql::predicate, rdf::tts("name"))));
			gtk_tree_store_set(store, &row, VLC_ENGINE, engine.c_str(), -1);
		}
		else if (rql::predicate(*statement) == rdf::dc("language"))
			gtk_tree_store_set(store, &row, VLC_LANGUAGE, languages(rql::value(*statement)).c_str(), -1);
		else if (rql::predicate(*statement) == rdf::tts("gender"))
		{
			if (rql::object(*statement) == rdf::tts("male"))
				gtk_tree_store_set(store, &row, VLC_GENDER, _("male"), -1);
			else if (rql::object(*statement) == rdf::tts("female"))
				gtk_tree_store_set(store, &row, VLC_GENDER, _("male"), -1);
		}
		else if (rql::predicate(*statement) == rdf::tts("age"))
			gtk_tree_store_set(store, &row, VLC_AGE, rql::value(*statement).c_str(), -1);
		else if (rql::predicate(*statement) == rdf::tts("frequency"))
			gtk_tree_store_set(store, &row, VLC_FREQUENCY, rql::value(*statement).c_str(), -1);
		else if (rql::predicate(*statement) == rdf::tts("channels"))
			gtk_tree_store_set(store, &row, VLC_CHANNELS, rql::value(*statement) == "1" ? _("mono") : _("stereo"), -1);
	}
}

VoiceSelectionView::VoiceSelectionView(tts::engines &aEngines, rdf::graph &aMetadata, cainteoir::languages &languages)
	: mEngines(&aEngines)
	, voices(aMetadata, languages)
	, parameterView(5, 3, false)
	, apply(_("_Apply"), true)
{
	set_border_width(6);

	voices_header.set_alignment(0, 0);
	voices_header.set_markup(_("<b>Voices</b>"));

	header.set_alignment(0, 0);
	header.set_markup(_("<b>Voice Settings</b>"));

	create_entry(tts::parameter::rate, 0);
	create_entry(tts::parameter::volume, 1);
	create_entry(tts::parameter::pitch, 2);
	create_entry(tts::parameter::pitch_range, 3);

	buttons.add(apply);
	buttons.set_layout(Gtk::BUTTONBOX_START);

	pack_start(voices_header, Gtk::PACK_SHRINK);
	gtk_box_pack_start(GTK_BOX(gobj()), voices, TRUE, TRUE, 12);
	pack_start(header, Gtk::PACK_SHRINK);
	pack_start(parameterView, Gtk::PACK_SHRINK, 12);
	pack_start(buttons, Gtk::PACK_SHRINK);

	apply.signal_clicked().connect(sigc::mem_fun(*this, &VoiceSelectionView::apply_settings));
}

void VoiceSelectionView::show()
{
	foreach_iter (item, parameters)
	{
		std::tr1::shared_ptr<tts::parameter> parameter = mEngines->parameter(item->type);

		item->param->set_range(parameter->minimum(), parameter->maximum());
		item->param->set_value(parameter->value());

		item->label->set_markup(parameter->name());
		item->units->set_markup(parameter->units());
	}

	Gtk::VBox::show();
}

void VoiceSelectionView::apply_settings()
{
	foreach_iter (item, parameters)
	{
		mEngines->parameter(item->type)->set_value(item->param->get_value());
	}
}

void VoiceSelectionView::create_entry(tts::parameter::type aParameter, int row)
{
	VoiceParameter item;
	item.type = aParameter;

	item.param = Gtk::manage(new Gtk::HScale());
	item.param->set_increments(1.0, 5.0);
	item.param->set_value_pos(Gtk::POS_RIGHT);
	item.param->set_digits(0);

	item.label = Gtk::manage(new Gtk::Label(""));
	item.label->set_alignment(0, 0.5);

	item.units = Gtk::manage(new Gtk::Label(""));
	item.units->set_alignment(0, 0.5);

	parameterView.attach(*item.label, 0, 1, row, row+1, Gtk::FILL, Gtk::FILL, 4, 4);
	parameterView.attach(*item.param, 1, 2, row, row+1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL, 4, 4);
	parameterView.attach(*item.units, 2, 3, row, row+1, Gtk::FILL, Gtk::FILL, 4, 4);

	parameters.push_back(item);
}
