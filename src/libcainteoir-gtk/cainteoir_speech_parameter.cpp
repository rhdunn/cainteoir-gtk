/* A GTK+ wrapper around the cainteoir::tts::parameter class.
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

#include <cainteoir-gtk/cainteoir_speech_parameter.h>

#include "cainteoir_speech_parameter_private.h"
#include "extensions/glib.h"

struct CainteoirSpeechParameterPrivate
{
	std::shared_ptr<cainteoir::tts::parameter> param;
};

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirSpeechParameter, cainteoir_speech_parameter, G_TYPE_OBJECT)

#define CAINTEOIR_SPEECH_PARAMETER_PRIVATE(object) \
	((CainteoirSpeechParameterPrivate *)cainteoir_speech_parameter_get_instance_private(CAINTEOIR_SPEECH_PARAMETER(object)))


GXT_DEFINE_TYPE_CONSTRUCTION(CainteoirSpeechParameter, cainteoir_speech_parameter, CAINTEOIR_SPEECH_PARAMETER)

static void
cainteoir_speech_parameter_class_init(CainteoirSpeechParameterClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = cainteoir_speech_parameter_finalize;
}

CainteoirSpeechParameter *
cainteoir_speech_parameter_new(const std::shared_ptr<cainteoir::tts::parameter> &parameter)
{
	CainteoirSpeechParameter *self = CAINTEOIR_SPEECH_PARAMETER(g_object_new(CAINTEOIR_TYPE_SPEECH_PARAMETER, nullptr));
	CainteoirSpeechParameterPrivate *priv = CAINTEOIR_SPEECH_PARAMETER_PRIVATE(self);
	priv->param = parameter;
	return self;
}

const gchar *
cainteoir_speech_parameter_get_name(CainteoirSpeechParameter *parameter)
{
	return CAINTEOIR_SPEECH_PARAMETER_PRIVATE(parameter)->param->name();
}

const gchar *
cainteoir_speech_parameter_get_units(CainteoirSpeechParameter *parameter)
{
	return CAINTEOIR_SPEECH_PARAMETER_PRIVATE(parameter)->param->units();
}

gint
cainteoir_speech_parameter_get_minimum(CainteoirSpeechParameter *parameter)
{
	return CAINTEOIR_SPEECH_PARAMETER_PRIVATE(parameter)->param->minimum();
}

gint
cainteoir_speech_parameter_get_maximum(CainteoirSpeechParameter *parameter)
{
	return CAINTEOIR_SPEECH_PARAMETER_PRIVATE(parameter)->param->maximum();
}

gint
cainteoir_speech_parameter_get_default(CainteoirSpeechParameter *parameter)
{
	return CAINTEOIR_SPEECH_PARAMETER_PRIVATE(parameter)->param->default_value();
}

gint
cainteoir_speech_parameter_get_value(CainteoirSpeechParameter *parameter)
{
	return CAINTEOIR_SPEECH_PARAMETER_PRIVATE(parameter)->param->value();
}

gboolean
cainteoir_speech_parameter_set_value(CainteoirSpeechParameter *parameter,
                                     gint value)
{
	return CAINTEOIR_SPEECH_PARAMETER_PRIVATE(parameter)->param->set_value(value);
}
