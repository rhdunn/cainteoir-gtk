/* Display the available speech synthesis voices object.
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

#include <cainteoir-gtk/cainteoir_speech_voice_view.h>

#include "cainteoir_speech_synthesizers_private.h"

#include <cainteoir/locale.hpp>

namespace rdf = cainteoir::rdf;
namespace rql = cainteoir::rdf::query;

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

struct _CainteoirSpeechVoiceViewPrivate
{
	GtkTreeStore *store;

	CainteoirSpeechSynthesizers *tts;

	cainteoir::languages languages;
	cainteoir::language::tag filter_language;

	_CainteoirSpeechVoiceViewPrivate();
};

_CainteoirSpeechVoiceViewPrivate::_CainteoirSpeechVoiceViewPrivate()
	: filter_language({})
{
}

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirSpeechVoiceView, cainteoir_speech_voice_view, GTK_TYPE_TREE_VIEW)

static void
cainteoir_speech_voice_view_finalize(GObject *object)
{
	CainteoirSpeechVoiceView *view = CAINTEOIR_SPEECH_VOICE_VIEW(object);
	if (view->priv->tts) g_object_unref(view->priv->tts);
	view->priv->~CainteoirSpeechVoiceViewPrivate();

	G_OBJECT_CLASS(cainteoir_speech_voice_view_parent_class)->finalize(object);
}

static void
cainteoir_speech_voice_view_class_init(CainteoirSpeechVoiceViewClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = cainteoir_speech_voice_view_finalize;
}

static void
cainteoir_speech_voice_view_init(CainteoirSpeechVoiceView *view)
{
	void *data = cainteoir_speech_voice_view_get_instance_private(view);
	view->priv = new (data)CainteoirSpeechVoiceViewPrivate();
	view->priv->tts = nullptr;
}

static void
refresh_view(CainteoirSpeechVoiceView *view)
{
	rdf::graph &metadata = *cainteoir_speech_synthesizers_get_metadata(view->priv->tts);

	gtk_tree_store_clear(view->priv->store);
	for (auto &voice : rql::select(metadata,
	                               rql::predicate == rdf::rdf("type") && rql::object == rdf::tts("Voice")))
	{
		rql::results statements = rql::select(metadata, rql::subject == rql::subject(voice));

		if (!view->priv->filter_language.lang.empty())
		{
			std::string lang = rql::select_value<std::string>(statements,
			                   rql::predicate == rdf::dc("language"));

			if (!(cainteoir::language::make_lang(lang) == view->priv->filter_language))
				continue;
		}

		GtkTreeIter row;
		gtk_tree_store_append(view->priv->store, &row, nullptr);

		for (auto &statement : statements)
		{
			if (rql::predicate(statement) == rdf::tts("name"))
				gtk_tree_store_set(view->priv->store, &row,
					VLC_URI,   rql::subject(statement).str().c_str(),
					VLC_VOICE, rql::value(statement).c_str(),
					-1);
			else if (rql::predicate(statement) == rdf::tts("voiceOf"))
			{
				std::string engine = rql::select_value<std::string>(metadata,
				                     rql::subject   == rql::object(statement) &&
				                     rql::predicate == rdf::tts("name"));
				gtk_tree_store_set(view->priv->store, &row, VLC_ENGINE, engine.c_str(), -1);
			}
			else if (rql::predicate(statement) == rdf::dc("language"))
			{
				cainteoir::language::tag lang = cainteoir::language::make_lang(rql::value(statement));
				gtk_tree_store_set(view->priv->store, &row, VLC_LANGUAGE, view->priv->languages.language(lang).c_str(), -1);
				gtk_tree_store_set(view->priv->store, &row, VLC_REGION, view->priv->languages.region(lang).c_str(), -1);
			}
			else if (rql::predicate(statement) == rdf::tts("gender"))
			{
				if (rql::object(statement) == rdf::tts("male"))
					gtk_tree_store_set(view->priv->store, &row, VLC_GENDER, i18n("male"), -1);
				else if (rql::object(statement) == rdf::tts("female"))
					gtk_tree_store_set(view->priv->store, &row, VLC_GENDER, i18n("female"), -1);
			}
			else if (rql::predicate(statement) == rdf::tts("frequency"))
				gtk_tree_store_set(view->priv->store, &row, VLC_FREQUENCY, rql::value(statement).c_str(), -1);
			else if (rql::predicate(statement) == rdf::tts("channels"))
				gtk_tree_store_set(view->priv->store, &row,
				                   VLC_CHANNELS, rql::value(statement) == "1" ? i18n("mono") : i18n("stereo"),
				                   -1);
		}
	}
}

GtkWidget *
cainteoir_speech_voice_view_new(CainteoirSpeechSynthesizers *synthesizers)
{
	CainteoirSpeechVoiceView *view = CAINTEOIR_SPEECH_VOICE_VIEW(g_object_new(CAINTEOIR_TYPE_SPEECH_VOICE_VIEW, nullptr));
	view->priv->tts = CAINTEOIR_SPEECH_SYNTHESIZERS(g_object_ref(G_OBJECT(synthesizers)));

	view->priv->store = gtk_tree_store_new(VLC_COUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(view), GTK_TREE_MODEL(view->priv->store));

	for (int i = 0; i < VLC_COUNT; ++i)
	{
		if (i == VLC_URI) continue;

		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
		GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(gettext(columns[i]), renderer, "text", i, nullptr);
		gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
		gtk_tree_view_column_set_sort_column_id(column, i);
	}

	refresh_view(view);

	return GTK_WIDGET(view);
}

void
cainteoir_speech_voice_view_filter_by_language(CainteoirSpeechVoiceView *view,
                                               const gchar *language)
{
	if (language)
		view->priv->filter_language = cainteoir::language::make_lang(language);
	else
		view->priv->filter_language = {{}};

	refresh_view(view);
}
