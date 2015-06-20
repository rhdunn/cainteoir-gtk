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
namespace events = cainteoir::events;

struct text_range_t
{
	CainteoirSpeechSynthesizers *synthesizers;
	gint text_start;
	gint text_end;
};

enum
{
	SPEAKING,
	TEXT_RANGE_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static gboolean
on_text_range_changed(text_range_t *range)
{
	g_signal_emit(range->synthesizers, signals[TEXT_RANGE_CHANGED], 0, range->text_start, range->text_end);
	g_free(range);
	return FALSE;
}

struct CainteoirSpeechSynthesizersPrivate : public tts::synthesis_callback
{
	CainteoirSpeechSynthesizers *self;

	rdf::graph metadata;
	tts::engines tts;
	tts::media_overlays_mode narration;
	CainteoirTextEventMode text_event_mode;
	CainteoirTextEventMode speak_mode;

	std::shared_ptr<cainteoir::tts::speech> speech;
	std::shared_ptr<cainteoir::audio> out;
	gchar *device_name;

	gint offset;
	bool need_linebreak;

	CainteoirSpeechSynthesizersPrivate()
		: tts(metadata)
		, narration(tts::media_overlays_mode::tts_only)
		, text_event_mode(CAINTEOIR_TEXT_EVENT_NONE)
		, speak_mode(CAINTEOIR_TEXT_EVENT_NONE)
		, device_name(nullptr)
		, offset(-1)
		, need_linebreak(false)
	{
	}

	~CainteoirSpeechSynthesizersPrivate()
	{
		if (speech) speech->stop();

		g_free(device_name);
	}

	tts::state_t state() const;

	void onaudiodata(short *data, int nsamples);

	void ontextrange(const cainteoir::range<uint32_t> &range);

	void onevent(const cainteoir::document_item &item);
};

tts::state_t CainteoirSpeechSynthesizersPrivate::state() const
{
	return tts::stopped;
}

void CainteoirSpeechSynthesizersPrivate::onaudiodata(short *data, int nsamples)
{
}

void CainteoirSpeechSynthesizersPrivate::ontextrange(const cainteoir::range<uint32_t> &range)
{
	if (text_event_mode == speak_mode || text_event_mode == CAINTEOIR_TEXT_EVENT_BOTH)
	{
		text_range_t *param = g_new(text_range_t, 1);
		param->synthesizers = self;
		param->text_start   = (gint)range.begin()+offset;
		param->text_end     = (gint)range.end()+offset;
		g_idle_add((GSourceFunc)on_text_range_changed, param);
	}
}

void CainteoirSpeechSynthesizersPrivate::onevent(const cainteoir::document_item &item)
{
	if (item.type & events::begin_context)
	{
		if (need_linebreak && item.styles) switch (item.styles->display)
		{
		case cainteoir::css::display::block:
		case cainteoir::css::display::line_break:
		case cainteoir::css::display::list_item:
		case cainteoir::css::display::table:
		case cainteoir::css::display::table_row:
		case cainteoir::css::display::table_cell:
			++offset; // Track the '\n' characters.
			need_linebreak = false;
			break;
		}
	}
	if (item.type & cainteoir::events::text)
	{
		need_linebreak = true;
	}
}

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirSpeechSynthesizers, cainteoir_speech_synthesizers, G_TYPE_OBJECT)

#define CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(object) \
	((CainteoirSpeechSynthesizersPrivate *)cainteoir_speech_synthesizers_get_instance_private(CAINTEOIR_SPEECH_SYNTHESIZERS(object)))

GXT_DEFINE_TYPE_CONSTRUCTION(CainteoirSpeechSynthesizers, cainteoir_speech_synthesizers, CAINTEOIR_SPEECH_SYNTHESIZERS)

static void
cainteoir_speech_synthesizers_class_init(CainteoirSpeechSynthesizersClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = cainteoir_speech_synthesizers_finalize;

	signals[SPEAKING] =
		g_signal_new("speaking",
		             G_TYPE_FROM_CLASS(object),
		             G_SIGNAL_RUN_LAST,
		             0,
		             NULL,
		             NULL,
		             g_cclosure_marshal_VOID__BOOLEAN,
		             G_TYPE_NONE,
		             1,
		             G_TYPE_BOOLEAN);

	signals[TEXT_RANGE_CHANGED] =
		g_signal_new("text-range-changed",
		             G_TYPE_FROM_CLASS(object),
		             G_SIGNAL_RUN_LAST,
		             0,
		             NULL,
		             NULL,
		             g_cclosure_marshal_generic,
		             G_TYPE_NONE,
		             2,
		             G_TYPE_INT, G_TYPE_INT);
}

static gboolean
on_speaking(CainteoirSpeechSynthesizersPrivate *priv)
{
	if (priv->speech->is_speaking())
	{
		g_signal_emit(priv->self, signals[SPEAKING], 0, TRUE);
		return TRUE;
	}

	g_signal_emit(priv->self, signals[SPEAKING], 0, FALSE);
	return FALSE;
}

static void
cainteoir_speech_synthesizers_speak(CainteoirSpeechSynthesizersPrivate *priv,
                                    CainteoirDocument *document,
                                    CainteoirDocumentIndex *index)
{
	auto doc = cainteoir_document_get_document(document);
	auto sel = cainteoir_document_index_get_selection_uri(CAINTEOIR_DOCUMENT_INDEX(index));
	const std::vector<cainteoir::ref_entry> &listing = *cainteoir_document_index_get_listing(CAINTEOIR_DOCUMENT_INDEX(index));

	priv->offset = -1;
	priv->need_linebreak = false;
	priv->speech = priv->tts.speak(priv->out, listing, *doc, doc->children(sel), priv->narration, priv);

	g_timeout_add(100, (GSourceFunc)on_speaking, priv);
}

CainteoirSpeechSynthesizers *
cainteoir_speech_synthesizers_new()
{
	CainteoirSpeechSynthesizers *synthesizers = CAINTEOIR_SPEECH_SYNTHESIZERS(g_object_new(CAINTEOIR_TYPE_SPEECH_SYNTHESIZERS, nullptr));
	CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(synthesizers)->self = synthesizers;
	return synthesizers;
}

gboolean
cainteoir_speech_synthesizers_set_voice(CainteoirSpeechSynthesizers *synthesizers,
                                        const gchar *voice)
{
	CainteoirSpeechSynthesizersPrivate *priv = CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(synthesizers);
	return priv->tts.select_voice(priv->metadata, rdf::href(voice));
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

gchar *
cainteoir_speech_synthesizers_get_voice(CainteoirSpeechSynthesizers *synthesizers)
{
	CainteoirSpeechSynthesizersPrivate *priv = CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(synthesizers);
	return g_strdup(priv->tts.voice().str().c_str());
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

CainteoirTextEventMode
cainteoir_speech_synthesizers_get_text_event_mode(CainteoirSpeechSynthesizers *synthesizers)
{
	return CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(synthesizers)->text_event_mode;
}

void
cainteoir_speech_synthesizers_set_text_event_mode(CainteoirSpeechSynthesizers *synthesizers,
                                                  CainteoirTextEventMode mode)
{
	CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(synthesizers)->text_event_mode = mode;
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

		priv->speak_mode = CAINTEOIR_TEXT_EVENT_WHILE_READING;
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
                                     const gchar *mimetype,
                                     gfloat quality)
{
	if (cainteoir_speech_synthesizers_is_speaking(synthesizers))
		return;

	try
	{
		CainteoirSpeechSynthesizersPrivate *priv = CAINTEOIR_SPEECH_SYNTHESIZERS_PRIVATE(synthesizers);
		g_free(priv->device_name);
		priv->device_name = g_strdup(filename);

		if (!strcmp(mimetype, "audio/x-wav") ||
		    !strcmp(mimetype, "audio/vnd.wave") ||
		    !strcmp(mimetype, "audio/wav"))
			priv->out = cainteoir::create_wav_file(
				priv->device_name,
				priv->metadata,
				priv->tts.voice());
		else if (!strcmp(mimetype, "audio/x-vorbis+ogg") ||
		         !strcmp(mimetype, "audio/vorbis") ||
		         !strcmp(mimetype, "audio/x-vorbis"))
		{
			std::list<cainteoir::vorbis_comment> comments;
			cainteoir::add_document_metadata(comments,
				*cainteoir_document_get_rdf_metadata(doc),
				*cainteoir_document_get_subject(doc));
			priv->out = cainteoir::create_ogg_file(
				priv->device_name,
				comments,
				quality,
				priv->metadata,
				priv->tts.voice());
		}

		if (priv->out)
		{
			priv->speak_mode = CAINTEOIR_TEXT_EVENT_WHILE_RECORDING;
			cainteoir_speech_synthesizers_speak(priv, doc, index);
		}
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
