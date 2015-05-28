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
#include <cainteoir-gtk/cainteoir_speech_synthesizers.h>
#include <cainteoir-gtk/cainteoir_document.h>
#include <cainteoir-gtk/cainteoir_metadata.h>

#include "cainteoir_speech_synthesizers_private.h"
#include "extensions/glib.h"

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

struct VoiceListSelection
{
	const gchar *voice;
	GtkTreeSelection *selection;
};

static gboolean
voice_list_selection_select_voice(GtkTreeModel *model,
                                  GtkTreePath *path,
                                  GtkTreeIter *iter,
                                  VoiceListSelection *select)
{
	gchar *voice = nullptr;
	gtk_tree_model_get(model, iter, VLC_URI, &voice, -1);
	gboolean ret = !strcmp(select->voice, voice);
	if (ret)
		gtk_tree_selection_select_iter(select->selection, iter);
	g_free(voice);
	return ret;
}

enum
{
	VOICE_CHANGED,
	LAST_SIGNAL
};

static guint view_signals[LAST_SIGNAL] = { 0 };

struct CainteoirSpeechVoiceViewPrivate
{
	GtkTreeStore *store;
	GtkTreeSelection *selection;
	gulong signal;

	CainteoirSpeechSynthesizers *tts;

	cainteoir::languages languages;
	cainteoir::language::tag filter_language;

	CainteoirSpeechVoiceViewPrivate()
		: filter_language({})
		, signal((gulong)-1)
		, tts(nullptr)
	{
	}

	~CainteoirSpeechVoiceViewPrivate()
	{
		if (tts) g_object_unref(tts);
	}
};

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirSpeechVoiceView, cainteoir_speech_voice_view, GTK_TYPE_TREE_VIEW)

#define CAINTEOIR_SPEECH_VOICE_VIEW_PRIVATE(object) \
	((CainteoirSpeechVoiceViewPrivate *)cainteoir_speech_voice_view_get_instance_private(CAINTEOIR_SPEECH_VOICE_VIEW(object)))

GXT_DEFINE_TYPE_CONSTRUCTION(CainteoirSpeechVoiceView, cainteoir_speech_voice_view, CAINTEOIR_SPEECH_VOICE_VIEW)

static void
cainteoir_speech_voice_view_class_init(CainteoirSpeechVoiceViewClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = cainteoir_speech_voice_view_finalize;

	view_signals[VOICE_CHANGED] =
		g_signal_new("voice-changed",
		             G_TYPE_FROM_CLASS(object),
		             G_SIGNAL_RUN_LAST,
		             0,
		             NULL,
		             NULL,
		             g_cclosure_marshal_VOID__POINTER,
		             G_TYPE_NONE,
		             1,
		             G_TYPE_STRING);
}

static void
on_voice_selection_changed(GtkTreeSelection *selection, CainteoirSpeechVoiceView *view)
{
	CainteoirSpeechVoiceViewPrivate *priv = CAINTEOIR_SPEECH_VOICE_VIEW_PRIVATE(view);
	GtkTreeIter row;
	if (gtk_tree_selection_get_selected(priv->selection, nullptr, &row))
	{
		gchar *voice = nullptr;
		gtk_tree_model_get(GTK_TREE_MODEL(priv->store), &row, VLC_URI, &voice, -1);
		g_signal_emit(view, view_signals[VOICE_CHANGED], 0, voice);
		g_free(voice);
	}
}

static void
refresh_view(CainteoirSpeechVoiceView *view)
{
	CainteoirSpeechVoiceViewPrivate *priv = CAINTEOIR_SPEECH_VOICE_VIEW_PRIVATE(view);
	rdf::graph &metadata = *cainteoir_speech_synthesizers_get_metadata(priv->tts);

	gchar *voice = cainteoir_speech_voice_view_get_voice(view);
	if (priv->signal != (gulong)-1) g_signal_handler_disconnect(priv->selection, priv->signal);

	gtk_tree_store_clear(priv->store);
	for (auto &voice : rql::select(metadata,
	                               rql::predicate == rdf::rdf("type") && rql::object == rdf::tts("Voice")))
	{
		rql::results statements = rql::select(metadata, rql::subject == rql::subject(voice));

		if (!priv->filter_language.lang.empty())
		{
			std::string lang = rql::select_value<std::string>(statements,
			                   rql::predicate == rdf::dc("language"));

			if (!(cainteoir::language::make_lang(lang) == priv->filter_language))
				continue;
		}

		GtkTreeIter row;
		gtk_tree_store_append(priv->store, &row, nullptr);

		for (auto &statement : statements)
		{
			if (rql::predicate(statement) == rdf::tts("name"))
				gtk_tree_store_set(priv->store, &row,
					VLC_URI,   rql::subject(statement).str().c_str(),
					VLC_VOICE, rql::value(statement).c_str(),
					-1);
			else if (rql::predicate(statement) == rdf::tts("voiceOf"))
			{
				std::string engine = rql::select_value<std::string>(metadata,
				                     rql::subject   == rql::object(statement) &&
				                     rql::predicate == rdf::tts("name"));
				gtk_tree_store_set(priv->store, &row, VLC_ENGINE, engine.c_str(), -1);
			}
			else if (rql::predicate(statement) == rdf::dc("language"))
			{
				cainteoir::language::tag lang = cainteoir::language::make_lang(rql::value(statement));
				gtk_tree_store_set(priv->store, &row, VLC_LANGUAGE, priv->languages.language(lang).c_str(), -1);
				gtk_tree_store_set(priv->store, &row, VLC_REGION, priv->languages.region(lang).c_str(), -1);
			}
			else if (rql::predicate(statement) == rdf::tts("gender"))
			{
				if (rql::object(statement) == rdf::tts("male"))
					gtk_tree_store_set(priv->store, &row, VLC_GENDER, i18n("male"), -1);
				else if (rql::object(statement) == rdf::tts("female"))
					gtk_tree_store_set(priv->store, &row, VLC_GENDER, i18n("female"), -1);
			}
			else if (rql::predicate(statement) == rdf::tts("frequency"))
				gtk_tree_store_set(priv->store, &row, VLC_FREQUENCY, rql::value(statement).c_str(), -1);
			else if (rql::predicate(statement) == rdf::tts("channels"))
				gtk_tree_store_set(priv->store, &row,
				                   VLC_CHANNELS, rql::value(statement) == "1" ? i18n("mono") : i18n("stereo"),
				                   -1);
		}
	}

	if (voice)
	{
		cainteoir_speech_voice_view_set_voice(view, voice);
		g_free(voice);
	}

	priv->signal = g_signal_connect(priv->selection, "changed", G_CALLBACK(on_voice_selection_changed), view);
}

GtkWidget *
cainteoir_speech_voice_view_new(CainteoirSpeechSynthesizers *synthesizers)
{
	CainteoirSpeechVoiceView *view = CAINTEOIR_SPEECH_VOICE_VIEW(g_object_new(CAINTEOIR_TYPE_SPEECH_VOICE_VIEW, nullptr));
	CainteoirSpeechVoiceViewPrivate *priv = CAINTEOIR_SPEECH_VOICE_VIEW_PRIVATE(view);
	priv->tts = CAINTEOIR_SPEECH_SYNTHESIZERS(g_object_ref(G_OBJECT(synthesizers)));

	priv->store = gtk_tree_store_new(VLC_COUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(view), GTK_TREE_MODEL(priv->store));

	for (int i = 0; i < VLC_COUNT; ++i)
	{
		if (i == VLC_URI) continue;

		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
		GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(gettext(columns[i]), renderer, "text", i, nullptr);
		gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
		gtk_tree_view_column_set_sort_column_id(column, i);
	}

	priv->selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	refresh_view(view);

	return GTK_WIDGET(view);
}

void
cainteoir_speech_voice_view_set_filter_language(CainteoirSpeechVoiceView *view,
                                                const gchar *language)
{
	CainteoirSpeechVoiceViewPrivate *priv = CAINTEOIR_SPEECH_VOICE_VIEW_PRIVATE(view);
	if (language)
		priv->filter_language = cainteoir::language::make_lang(language);
	else
		priv->filter_language = {{}};

	refresh_view(view);
}

void
cainteoir_speech_voice_view_set_filter_language_from_document(CainteoirSpeechVoiceView *view,
                                                              CainteoirDocument *document)
{
	if (!document)
		cainteoir_speech_voice_view_set_filter_language(view, nullptr);
	else
	{
		CainteoirMetadata *metadata = cainteoir_document_get_metadata(document);
		gchar *language = cainteoir_metadata_get_string(metadata, CAINTEOIR_METADATA_LANGUAGE);

		cainteoir_speech_voice_view_set_filter_language(view, language);

		g_free(language);
		g_object_unref(G_OBJECT(metadata));
	}
}

gchar *
cainteoir_speech_voice_view_get_voice(CainteoirSpeechVoiceView *view)
{
	CainteoirSpeechVoiceViewPrivate *priv = CAINTEOIR_SPEECH_VOICE_VIEW_PRIVATE(view);
	GtkTreeIter row;
	if (gtk_tree_selection_get_selected(priv->selection, nullptr, &row))
	{
		gchar *voice = nullptr;
		gtk_tree_model_get(GTK_TREE_MODEL(priv->store), &row, VLC_URI, &voice, -1);
		return voice;
	}
	return nullptr;
}

void
cainteoir_speech_voice_view_set_voice(CainteoirSpeechVoiceView *view,
                                      const gchar *voice)
{
	if (!voice) return;

	CainteoirSpeechVoiceViewPrivate *priv = CAINTEOIR_SPEECH_VOICE_VIEW_PRIVATE(view);
	VoiceListSelection select = { voice, priv->selection };
	gtk_tree_model_foreach(GTK_TREE_MODEL(priv->store),
	                       (GtkTreeModelForeachFunc)voice_list_selection_select_voice,
	                       &select);
}
