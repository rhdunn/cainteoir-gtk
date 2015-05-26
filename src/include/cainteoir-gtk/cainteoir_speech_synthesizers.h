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

#ifndef CAINTEOIR_GTK_CAINTEOIR_SPEECH_SYNTHESIZERS_H
#define CAINTEOIR_GTK_CAINTEOIR_SPEECH_SYNTHESIZERS_H

typedef struct _CainteoirDocument           CainteoirDocument;
typedef struct _CainteoirDocumentIndex      CainteoirDocumentIndex;
typedef struct _CainteoirSpeechParameter    CainteoirSpeechParameter;

G_BEGIN_DECLS

#define CAINTEOIR_TYPE_SPEECH_SYNTHESIZERS \
	(cainteoir_speech_synthesizers_get_type())
#define CAINTEOIR_SPEECH_SYNTHESIZERS(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), CAINTEOIR_TYPE_SPEECH_SYNTHESIZERS, CainteoirSpeechSynthesizers))
#define CAINTEOIR_SPEECH_SYNTHESIZERS_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), CAINTEOIR_TYPE_SPEECH_SYNTHESIZERS, CainteoirSpeechSynthesizersClass))
#define CAINTEOIR_IS_SPEECH_SYNTHESIZERS(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), CAINTEOIR_TYPE_SPEECH_SYNTHESIZERS))
#define CAINTEOIR_IS_SPEECH_SYNTHESIZERS_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), CAINTEOIR_TYPE_SPEECH_SYNTHESIZERS))
#define CAINTEOIR_SPEECH_SYNTHESIZERS_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), CAINTEOIR_TYPE_SPEECH_SYNTHESIZERS, CainteoirSpeechSynthesizersClass))

typedef struct _CainteoirSpeechSynthesizers        CainteoirSpeechSynthesizers;
typedef struct _CainteoirSpeechSynthesizersPrivate CainteoirSpeechSynthesizersPrivate;
typedef struct _CainteoirSpeechSynthesizersClass   CainteoirSpeechSynthesizersClass;

struct _CainteoirSpeechSynthesizers
{
	GObject parent_instance;

	/*< private >*/
	CainteoirSpeechSynthesizersPrivate *priv;
};

struct _CainteoirSpeechSynthesizersClass
{
	GObjectClass parent_class;

	/* Padding for future expansion */
	void (*_reserved1)(void);
	void (*_reserved2)(void);
	void (*_reserved3)(void);
	void (*_reserved4)(void);
};

enum CainteoirNarration
{
	CAINTEOIR_NARRATION_TTS_ONLY,
	CAINTEOIR_NARRATION_MEDIA_OVERLAYS_ONLY,
	CAINTEOIR_NARRATION_TTS_AND_MEDIA_OVERLAYS,
};

enum CainteoirSpeechParameterType
{
	CAINTEOIR_SPEECH_RATE,
	CAINTEOIR_SPEECH_VOLUME,
	CAINTEOIR_SPEECH_PITCH,
	CAINTEOIR_SPEECH_PITCH_RANGE,
	CAINTEOIR_SPEECH_WORD_GAP,
};

GType                                  cainteoir_speech_synthesizers_get_type(void) G_GNUC_CONST;

CainteoirSpeechSynthesizers *          cainteoir_speech_synthesizers_new();

gboolean                               cainteoir_speech_synthesizers_set_voice_by_language(CainteoirSpeechSynthesizers *synthesizers,
                                                                                           const gchar *lang);

CainteoirNarration                     cainteoir_speech_synthesizers_get_narration(CainteoirSpeechSynthesizers *synthesizers);

void                                   cainteoir_speech_synthesizers_set_narration(CainteoirSpeechSynthesizers *synthesizers,
                                                                                   CainteoirNarration narration);

void                                   cainteoir_speech_synthesizers_read(CainteoirSpeechSynthesizers *synthesizers,
                                                                          CainteoirDocument *doc,
                                                                          CainteoirDocumentIndex *index,
                                                                          const gchar *device_name);

void                                   cainteoir_speech_synthesizers_record(CainteoirSpeechSynthesizers *synthesizers,
                                                                            CainteoirDocument *doc,
                                                                            CainteoirDocumentIndex *index,
                                                                            const gchar *filename,
                                                                            const gchar *type,
                                                                            gfloat quality);

gboolean                               cainteoir_speech_synthesizers_is_speaking(CainteoirSpeechSynthesizers *synthesizers);

void                                   cainteoir_speech_synthesizers_stop(CainteoirSpeechSynthesizers *synthesizers);

gdouble                                cainteoir_speech_synthesizers_get_elapsed_time(CainteoirSpeechSynthesizers *synthesizers);

gdouble                                cainteoir_speech_synthesizers_get_total_time(CainteoirSpeechSynthesizers *synthesizers);

gdouble                                cainteoir_speech_synthesizers_get_percentage_complete(CainteoirSpeechSynthesizers *synthesizers);

size_t                                 cainteoir_speech_synthesizers_get_position(CainteoirSpeechSynthesizers *synthesizers);

CainteoirSpeechParameter *             cainteoir_speech_synthesizers_get_parameter(CainteoirSpeechSynthesizers *synthesizers,
                                                                                   CainteoirSpeechParameterType parameter);

G_END_DECLS

#endif
