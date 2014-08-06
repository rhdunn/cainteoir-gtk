/* A GTK+ wrapper around the cainteoir::tts::engines class.
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

#include <gtk/gtk.h>

#include <cainteoir-gtk/cainteoir_speech_synthesizers.h>
#include <cainteoir/engines.hpp>

namespace rdf = cainteoir::rdf;
namespace tts = cainteoir::tts;

struct _CainteoirSpeechSynthesizersPrivate
{
	rdf::graph metadata;
	tts::engines tts;

	_CainteoirSpeechSynthesizersPrivate();
};

_CainteoirSpeechSynthesizersPrivate::_CainteoirSpeechSynthesizersPrivate()
	: tts(metadata)
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

CainteoirSpeechSynthesizers *
cainteoir_speech_synthesizers_new()
{
	return CAINTEOIR_SPEECH_SYNTHESIZERS(g_object_new(CAINTEOIR_TYPE_SPEECH_SYNTHESIZERS, nullptr));
}
