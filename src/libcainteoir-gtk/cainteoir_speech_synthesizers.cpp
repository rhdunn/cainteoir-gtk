/* A GTK+ wrapper around the cainteoir::tts::engines class.
 *
 * Copyright (C) 2014-2015 Reece H. Dunn
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

#include <gtk/gtk.h>

#include <cainteoir-gtk/cainteoir_speech_synthesizers.h>
#include <cainteoir/engines.hpp>
#include <cainteoir/locale.hpp>

#include "cainteoir_document_private.h"
#include "cainteoir_document_index_private.h"

namespace rdf = cainteoir::rdf;
namespace rql = cainteoir::rdf::query;
namespace tts = cainteoir::tts;

struct _CainteoirSpeechSynthesizersPrivate
{
	rdf::graph metadata;
	tts::engines tts;
	tts::media_overlays_mode narration;

	std::shared_ptr<cainteoir::tts::speech> speech;
	std::shared_ptr<cainteoir::audio> out;

	_CainteoirSpeechSynthesizersPrivate();
};

_CainteoirSpeechSynthesizersPrivate::_CainteoirSpeechSynthesizersPrivate()
	: tts(metadata)
	, narration(tts::media_overlays_mode::tts_only)
{
}

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirSpeechSynthesizers, cainteoir_speech_synthesizers, G_TYPE_OBJECT)

static void
cainteoir_speech_synthesizers_finalize(GObject *object)
{
	CainteoirSpeechSynthesizers *doc = CAINTEOIR_SPEECH_SYNTHESIZERS(object);
	doc->priv->~CainteoirSpeechSynthesizersPrivate();

	G_OBJECT_CLASS(cainteoir_speech_synthesizers_parent_class)->finalize(object);
}

static void
cainteoir_speech_synthesizers_class_init(CainteoirSpeechSynthesizersClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = cainteoir_speech_synthesizers_finalize;
}

static void
cainteoir_speech_synthesizers_init(CainteoirSpeechSynthesizers *doc)
{
	void * data = cainteoir_speech_synthesizers_get_instance_private(doc);
	doc->priv = new (data)CainteoirSpeechSynthesizersPrivate();
}

static void
cainteoir_speech_synthesizers_speak(CainteoirSpeechSynthesizers *synthesizers,
                                    CainteoirDocument *document,
                                    CainteoirDocumentIndex *index)
{
	auto doc = cainteoir_document_get_document(document);
	auto sel = cainteoir_document_index_get_selection(CAINTEOIR_DOCUMENT_INDEX(index));
	const std::vector<cainteoir::ref_entry> &listing = *cainteoir_document_index_get_listing(CAINTEOIR_DOCUMENT_INDEX(index));
	synthesizers->priv->speech = synthesizers->priv->tts.speak(synthesizers->priv->out, listing, doc->children(sel), synthesizers->priv->narration);
}

CainteoirSpeechSynthesizers *
cainteoir_speech_synthesizers_new()
{
	return CAINTEOIR_SPEECH_SYNTHESIZERS(g_object_new(CAINTEOIR_TYPE_SPEECH_SYNTHESIZERS, nullptr));
}

gboolean
cainteoir_speech_synthesizers_set_voice_by_language(CainteoirSpeechSynthesizers *synthesizers,
                                                    const gchar *lang)
{
	auto language = cainteoir::language::make_lang(lang);

	// Does the current voice support this language? ...

	std::string current = rql::select_value<std::string>(synthesizers->priv->metadata,
	                      rql::subject == synthesizers->priv->tts.voice() && rql::predicate == rdf::dc("language"));

	if (cainteoir::language::make_lang(current) == language)
		return TRUE;

	// The current voice does not support this language, so search the available voices ...

	for (auto &voice : rql::select(synthesizers->priv->metadata,
	                               rql::predicate == rdf::rdf("type") && rql::object == rdf::tts("Voice")))
	{
		const rdf::uri &uri = rql::subject(voice);

		std::string lang = rql::select_value<std::string>(synthesizers->priv->metadata,
		                   rql::subject == uri && rql::predicate == rdf::dc("language"));

		if (cainteoir::language::make_lang(lang) == language &&
		    synthesizers->priv->tts.select_voice(synthesizers->priv->metadata, uri))
			return TRUE;
	}
	return FALSE;
}

CainteoirNarration
cainteoir_speech_synthesizers_get_narration(CainteoirSpeechSynthesizers *synthesizers)
{
	switch (synthesizers->priv->narration)
	{
	case tts::media_overlays_mode::tts_only:               return CAINTEOIR_NARRATION_TTS_ONLY;
	case tts::media_overlays_mode::media_overlays_only:    return CAINTEOIR_NARRATION_MEDIA_OVERLAYS_ONLY;
	case tts::media_overlays_mode::tts_and_media_overlays: return CAINTEOIR_NARRATION_TTS_AND_MEDIA_OVERLAYS;
	default:                                               return CAINTEOIR_NARRATION_TTS_ONLY;
	}
}

void
cainteoir_speech_synthesizers_set_narration(CainteoirSpeechSynthesizers *synthesizers,
                                            CainteoirNarration narration)
{
	switch (narration)
	{
	case CAINTEOIR_NARRATION_TTS_ONLY:
		synthesizers->priv->narration = tts::media_overlays_mode::tts_only;
		break;
	case CAINTEOIR_NARRATION_MEDIA_OVERLAYS_ONLY:
		synthesizers->priv->narration = tts::media_overlays_mode::media_overlays_only;
		break;
	case CAINTEOIR_NARRATION_TTS_AND_MEDIA_OVERLAYS:
		synthesizers->priv->narration = tts::media_overlays_mode::tts_and_media_overlays;
		break;
	}
}

void
cainteoir_speech_synthesizers_read(CainteoirSpeechSynthesizers *synthesizers,
                                   CainteoirDocument *doc,
                                   CainteoirDocumentIndex *index,
                                   const gchar *device_name)
{
	try
	{
		synthesizers->priv->out = cainteoir::open_audio_device(
			device_name,
			*cainteoir_document_get_rdf_metadata(doc),
			*cainteoir_document_get_subject(doc),
			synthesizers->priv->metadata,
			synthesizers->priv->tts.voice());

		cainteoir_speech_synthesizers_speak(synthesizers, doc, index);
	}
	catch (const std::exception &e)
	{
		fprintf(stderr, "error: %s\n", e.what());
	}
}
