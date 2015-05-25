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

#include <gtk/gtk.h>

#include <cainteoir-gtk/cainteoir_speech_voice_view.h>

struct _CainteoirSpeechVoiceViewPrivate
{
	CainteoirSpeechSynthesizers *tts;
};

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirSpeechVoiceView, cainteoir_speech_voice_view, GTK_TYPE_TREE_VIEW)

static void
cainteoir_speech_voice_view_finalize(GObject *object)
{
	CainteoirSpeechVoiceView *view = CAINTEOIR_SPEECH_VOICE_VIEW(object);
	if (view->priv->tts) g_object_unref(view->priv->tts);

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
	view->priv = (CainteoirSpeechVoiceViewPrivate *)cainteoir_speech_voice_view_get_instance_private(view);
	view->priv->tts = nullptr;
}

GtkWidget *
cainteoir_speech_voice_view_new(CainteoirSpeechSynthesizers *synthesizers)
{
	CainteoirSpeechVoiceView *view = CAINTEOIR_SPEECH_VOICE_VIEW(g_object_new(CAINTEOIR_TYPE_SPEECH_VOICE_VIEW, nullptr));
	view->priv->tts = CAINTEOIR_SPEECH_SYNTHESIZERS(g_object_ref(G_OBJECT(synthesizers)));
	return GTK_WIDGET(view);
}
