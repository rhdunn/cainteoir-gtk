/* Audio Data (Samples).
 *
 * Copyright (C) 2014 Reece H. Dunn
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

#include <cainteoir-gtk/cainteoir_audio_data.h>
#include <cainteoir/audio.hpp>

namespace rdf = cainteoir::rdf;

struct _CainteoirAudioDataPrivate
{
};

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirAudioData, cainteoir_audio_data, G_TYPE_OBJECT)

static void
cainteoir_audio_data_finalize(GObject *object)
{
	CainteoirAudioData *audio = CAINTEOIR_AUDIO_DATA(object);

	G_OBJECT_CLASS(cainteoir_audio_data_parent_class)->finalize(object);
}

static void
cainteoir_audio_data_class_init(CainteoirAudioDataClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = cainteoir_audio_data_finalize;
}

static void
cainteoir_audio_data_init(CainteoirAudioData *audio)
{
	audio->priv = (CainteoirAudioDataPrivate *)cainteoir_audio_data_get_instance_private(audio);
}

CainteoirAudioData *
cainteoir_audio_data_new(const char *filename)
{
	CainteoirAudioData *self = CAINTEOIR_AUDIO_DATA(g_object_new(CAINTEOIR_TYPE_AUDIO_DATA, nullptr));

	try
	{
		auto audio = cainteoir::create_media_reader(cainteoir::make_file_buffer(filename));
		if (!audio)
			throw std::runtime_error("unable to read the audio file");

		audio->set_target(audio);
	}
	catch (const std::exception &e)
	{
		fprintf(stderr, "error: %s\n", e.what());
		g_object_unref(self);
		return nullptr;
	}

	return self;
}
