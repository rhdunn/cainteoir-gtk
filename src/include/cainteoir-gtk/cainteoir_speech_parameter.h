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

#ifndef CAINTEOIR_GTK_CAINTEOIR_SPEECH_PARAMETER_H
#define CAINTEOIR_GTK_CAINTEOIR_SPEECH_PARAMETER_H

G_BEGIN_DECLS

#define CAINTEOIR_TYPE_SPEECH_PARAMETER \
	(cainteoir_speech_parameter_get_type())
#define CAINTEOIR_SPEECH_PARAMETER(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), CAINTEOIR_TYPE_SPEECH_PARAMETER, CainteoirSpeechParameter))
#define CAINTEOIR_SPEECH_PARAMETER_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), CAINTEOIR_TYPE_SPEECH_PARAMETER, CainteoirSpeechParameterClass))
#define CAINTEOIR_IS_SPEECH_PARAMETER(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), CAINTEOIR_TYPE_SPEECH_PARAMETER))
#define CAINTEOIR_IS_SPEECH_PARAMETER_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), CAINTEOIR_TYPE_SPEECH_PARAMETER))
#define CAINTEOIR_SPEECH_PARAMETER_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), CAINTEOIR_TYPE_SPEECH_PARAMETER, CainteoirSpeechParameterClass))

typedef struct _CainteoirSpeechParameter        CainteoirSpeechParameter;
typedef struct _CainteoirSpeechParameterPrivate CainteoirSpeechParameterPrivate;
typedef struct _CainteoirSpeechParameterClass   CainteoirSpeechParameterClass;

struct _CainteoirSpeechParameter
{
	GObject parent_instance;

	/*< private >*/
	CainteoirSpeechParameterPrivate *priv;
};

struct _CainteoirSpeechParameterClass
{
	GObjectClass parent_class;

	/* Padding for future expansion */
	void (*_reserved1)(void);
	void (*_reserved2)(void);
	void (*_reserved3)(void);
	void (*_reserved4)(void);
};

GType                                  cainteoir_speech_parameter_get_type(void) G_GNUC_CONST;

const gchar *                          cainteoir_speech_parameter_get_name(CainteoirSpeechParameter *parameter);

const gchar *                          cainteoir_speech_parameter_get_units(CainteoirSpeechParameter *parameter);

gint                                   cainteoir_speech_parameter_get_minimum(CainteoirSpeechParameter *parameter);

gint                                   cainteoir_speech_parameter_get_maximum(CainteoirSpeechParameter *parameter);

gint                                   cainteoir_speech_parameter_get_default(CainteoirSpeechParameter *parameter);

gint                                   cainteoir_speech_parameter_get_value(CainteoirSpeechParameter *parameter);

gboolean                               cainteoir_speech_parameter_set_value(CainteoirSpeechParameter *parameter,
                                                                            gint value);

G_END_DECLS

#endif
