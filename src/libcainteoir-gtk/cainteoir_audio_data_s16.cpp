/* Audio Data (Samples).
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

#include <glib-object.h>

#include <cainteoir-gtk/cainteoir_audio_data_s16.h>
#include <cainteoir/sigproc.hpp>

#include "extensions/glib.h"

namespace rdf = cainteoir::rdf;
namespace css = cainteoir::css;

typedef cainteoir::audio_data<short> audio_data_t;

typedef struct _CainteoirAudioDataS16Private CainteoirAudioDataS16Private;

struct _CainteoirAudioDataS16Private
{
	audio_data_t data;
	uint16_t frequency;
};

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirAudioDataS16, cainteoir_audio_data_s16, G_TYPE_OBJECT)

#define CAINTEOIR_AUDIO_DATA_S16_PRIVATE(object) \
	((CainteoirAudioDataS16Private *)cainteoir_audio_data_s16_get_instance_private(CAINTEOIR_AUDIO_DATA_S16(object)))

GXT_DEFINE_TYPE_CONSTRUCTION(CainteoirAudioDataS16, cainteoir_audio_data_s16, CAINTEOIR_AUDIO_DATA_S16)

static void
cainteoir_audio_data_s16_class_init(CainteoirAudioDataS16Class *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = cainteoir_audio_data_s16_finalize;
}

CainteoirAudioDataS16 *
cainteoir_audio_data_s16_new(const char *filename,
                             const char *start_time,
                             const char *end_time,
                             uint16_t channel,
                             uint16_t frequency)
{
	CainteoirAudioDataS16 *self = CAINTEOIR_AUDIO_DATA_S16(g_object_new(CAINTEOIR_TYPE_AUDIO_DATA_S16, nullptr));
	CainteoirAudioDataS16Private *priv = CAINTEOIR_AUDIO_DATA_S16_PRIVATE(self);

	try
	{
		auto audio = cainteoir::make_file_buffer(filename);
		if (!audio)
			throw std::runtime_error("unable to read the audio file");

		css::time start = css::parse_smil_time(start_time);
		css::time end   = css::parse_smil_time(end_time);

		priv->data = cainteoir::read_s16_samples(audio, start, end, channel, frequency);
	}
	catch (const std::exception &e)
	{
		fprintf(stderr, "error: %s\n", e.what());
		g_object_unref(self);
		return nullptr;
	}

	return self;
}

uint16_t
cainteoir_audio_data_s16_get_frequency(CainteoirAudioDataS16 *audio)
{
	return CAINTEOIR_AUDIO_DATA_S16_PRIVATE(audio)->data.info->frequency();
}

uint32_t
cainteoir_audio_data_s16_get_sample_count(CainteoirAudioDataS16 *audio)
{
	return CAINTEOIR_AUDIO_DATA_S16_PRIVATE(audio)->data.samples.size();
}

float
cainteoir_audio_data_s16_get_duration(CainteoirAudioDataS16 *audio)
{
	return (float)cainteoir_audio_data_s16_get_sample_count(audio) /
	       cainteoir_audio_data_s16_get_frequency(audio);
}

const short *
cainteoir_audio_data_s16_get_samples(CainteoirAudioDataS16 *audio)
{
	return &CAINTEOIR_AUDIO_DATA_S16_PRIVATE(audio)->data.samples[0];
}
