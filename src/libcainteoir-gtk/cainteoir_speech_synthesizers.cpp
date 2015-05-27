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
#include <cainteoir-gtk/cainteoir_document_index.h>
#include <cainteoir/engines.hpp>
#include <cainteoir/locale.hpp>

#include "cainteoir_document_private.h"
#include "cainteoir_document_index_private.h"
#include "cainteoir_speech_parameter_private.h"
#include "cainteoir_speech_synthesizers_private.h"
#include "extensions/glib.h"

namespace rdf = cainteoir::rdf;
namespace rql = cainteoir::rdf::query;
namespace tts = cainteoir::tts;

typedef struct _CainteoirSpeechSynthesizersPrivate CainteoirSpeechSynthesizersPrivate;

struct _CainteoirSpeechSynthesizersPrivate
{
	rdf::graph metadata;
	tts::engines tts;
	tts::media_overlays_mode narration;

	std::shared_ptr<cainteoir::tts::speech> speech;
	std::shared_ptr<cainteoir::audio> out;
	gchar *device_name;

	_CainteoirSpeechSynthesizersPrivate()
		: tts(metadata)
		, narration(tts::media_overlays_mode::tts_only)
		, device_name(nullptr)
	{
	}

	~_CainteoirSpeechSynthesizersPrivate()
	{
		if (speech) speech->stop();

		g_free(device_name);
	}
};

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirSpeechSynthesizers, cainteoir_speech_synthesizers, G_TYPE_OBJECT)

#define CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(object) \
	((CainteoirSpeechSynthesizersPrivate *)cainteoir_speech_synthesizers_get_instance_private(CAINTEOIR_SPEECH_SYNTHESIZERS(object)))

GXT_DEFINE_TYPE_CONSTRUCTION(CainteoirSpeechSynthesizers, cainteoir_speech_synthesizers, CAINTEOIR_SPEECH_SYNTHESIZERS)

static void
cainteoir_speech_synthesizers_class_init(CainteoirSpeechSynthesizersClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = cainteoir_speech_synthesizers_finalize;
}

static void
cainteoir_speech_synthesizers_speak(CainteoirSpeechSynthesizersPrivate *priv,
                                    CainteoirDocument *document,
                                    CainteoirDocumentIndex *index)
{
	auto doc = cainteoir_document_get_document(document);
	auto sel = cainteoir_document_index_get_selection(CAINTEOIR_DOCUMENT_INDEX(index));
	const std::vector<cainteoir::ref_entry> &listing = *cainteoir_document_index_get_listing(CAINTEOIR_DOCUMENT_INDEX(index));
	priv->speech = priv->tts.speak(priv->out, listing, doc->children(sel), priv->narration);
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
	CainteoirSpeechSynthesizersPrivate *priv = CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(synthesizers);
	auto language = cainteoir::language::make_lang(lang);

	// Does the current voice support this language? ...

	std::string current = rql::select_value<std::string>(priv->metadata,
	                      rql::subject == priv->tts.voice() && rql::predicate == rdf::dc("language"));

	if (cainteoir::language::make_lang(current) == language)
		return TRUE;

	// The current voice does not support this language, so search the available voices ...

	for (auto &voice : rql::select(priv->metadata,
	                               rql::predicate == rdf::rdf("type") && rql::object == rdf::tts("Voice")))
	{
		const rdf::uri &uri = rql::subject(voice);

		std::string lang = rql::select_value<std::string>(priv->metadata,
		                   rql::subject == uri && rql::predicate == rdf::dc("language"));

		if (cainteoir::language::make_lang(lang) == language &&
		    priv->tts.select_voice(priv->metadata, uri))
			return TRUE;
	}
	return FALSE;
}

CainteoirNarration
cainteoir_speech_synthesizers_get_narration(CainteoirSpeechSynthesizers *synthesizers)
{
	switch (CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(synthesizers)->narration)
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
	CainteoirSpeechSynthesizersPrivate *priv = CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(synthesizers);
	switch (narration)
	{
	case CAINTEOIR_NARRATION_TTS_ONLY:
		priv->narration = tts::media_overlays_mode::tts_only;
		break;
	case CAINTEOIR_NARRATION_MEDIA_OVERLAYS_ONLY:
		priv->narration = tts::media_overlays_mode::media_overlays_only;
		break;
	case CAINTEOIR_NARRATION_TTS_AND_MEDIA_OVERLAYS:
		priv->narration = tts::media_overlays_mode::tts_and_media_overlays;
		break;
	}
}

void
cainteoir_speech_synthesizers_read(CainteoirSpeechSynthesizers *synthesizers,
                                   CainteoirDocument *doc,
                                   CainteoirDocumentIndex *index,
                                   const gchar *device_name)
{
	if (cainteoir_speech_synthesizers_is_speaking(synthesizers))
		return;

	try
	{
		CainteoirSpeechSynthesizersPrivate *priv = CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(synthesizers);
		g_free(priv->device_name);
		priv->device_name = g_strdup(device_name);

		priv->out = cainteoir::open_audio_device(
			priv->device_name,
			*cainteoir_document_get_rdf_metadata(doc),
			*cainteoir_document_get_subject(doc),
			priv->metadata,
			priv->tts.voice());

		cainteoir_speech_synthesizers_speak(priv, doc, index);
	}
	catch (const std::exception &e)
	{
		fprintf(stderr, "error: %s\n", e.what());
	}
}

void
cainteoir_speech_synthesizers_record(CainteoirSpeechSynthesizers *synthesizers,
                                     CainteoirDocument *doc,
                                     CainteoirDocumentIndex *index,
                                     const gchar *filename,
                                     const gchar *type,
                                     gfloat quality)
{
	if (cainteoir_speech_synthesizers_is_speaking(synthesizers))
		return;

	try
	{
		CainteoirSpeechSynthesizersPrivate *priv = CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(synthesizers);
		g_free(priv->device_name);
		priv->device_name = g_strdup(filename);

		priv->out = cainteoir::create_audio_file(
			priv->device_name,
			type,
			quality,
			*cainteoir_document_get_rdf_metadata(doc),
			*cainteoir_document_get_subject(doc),
			priv->metadata,
			priv->tts.voice());

		cainteoir_speech_synthesizers_speak(priv, doc, index);
	}
	catch (const std::exception &e)
	{
		fprintf(stderr, "error: %s\n", e.what());
	}
}

gboolean
cainteoir_speech_synthesizers_is_speaking(CainteoirSpeechSynthesizers *synthesizers)
{
	CainteoirSpeechSynthesizersPrivate *priv = CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(synthesizers);
	if (!priv->speech) return FALSE;
	return priv->speech->is_speaking();
}

void
cainteoir_speech_synthesizers_stop(CainteoirSpeechSynthesizers *synthesizers)
{
	CainteoirSpeechSynthesizersPrivate *priv = CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(synthesizers);
	if (!priv->speech) return;
	priv->speech->stop();
}

gdouble
cainteoir_speech_synthesizers_get_elapsed_time(CainteoirSpeechSynthesizers *synthesizers)
{
	CainteoirSpeechSynthesizersPrivate *priv = CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(synthesizers);
	if (!priv->speech) return 0.0;
	return priv->speech->elapsedTime();
}

gdouble
cainteoir_speech_synthesizers_get_total_time(CainteoirSpeechSynthesizers *synthesizers)
{
	CainteoirSpeechSynthesizersPrivate *priv = CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(synthesizers);
	if (!priv->speech) return 0.0;
	return priv->speech->totalTime();
}

gdouble
cainteoir_speech_synthesizers_get_percentage_complete(CainteoirSpeechSynthesizers *synthesizers)
{
	CainteoirSpeechSynthesizersPrivate *priv = CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(synthesizers);
	if (!priv->speech) return 0.0;
	return priv->speech->completed();
}

size_t
cainteoir_speech_synthesizers_get_position(CainteoirSpeechSynthesizers *synthesizers)
{
	CainteoirSpeechSynthesizersPrivate *priv = CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(synthesizers);
	if (!priv->speech) return 0;
	return priv->speech->position();
}

CainteoirSpeechParameter *
cainteoir_speech_synthesizers_get_parameter(CainteoirSpeechSynthesizers *synthesizers,
                                            CainteoirSpeechParameterType parameter)
{
	tts::parameter::type type;
	switch (parameter)
	{
	case CAINTEOIR_SPEECH_RATE:        type = tts::parameter::rate;        break;
	case CAINTEOIR_SPEECH_VOLUME:      type = tts::parameter::volume;      break;
	case CAINTEOIR_SPEECH_PITCH:       type = tts::parameter::pitch;       break;
	case CAINTEOIR_SPEECH_PITCH_RANGE: type = tts::parameter::pitch_range; break;
	case CAINTEOIR_SPEECH_WORD_GAP:    type = tts::parameter::word_gap;    break;
	default:                           return nullptr;
	}
	return cainteoir_speech_parameter_new(CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(synthesizers)->tts.parameter(type));
}

rdf::graph *
cainteoir_speech_synthesizers_get_metadata(CainteoirSpeechSynthesizers *synthesizers)
{
	return &CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(synthesizers)->metadata;
}
