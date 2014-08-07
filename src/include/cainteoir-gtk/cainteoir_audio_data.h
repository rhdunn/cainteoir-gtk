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

#ifndef CAINTEOIR_GTK_CAINTEOIR_AUDIO_DATA_H
#define CAINTEOIR_GTK_CAINTEOIR_AUDIO_DATA_H

#include <stdint.h>

G_BEGIN_DECLS

#define CAINTEOIR_TYPE_AUDIO_DATA \
	(cainteoir_audio_data_get_type())
#define CAINTEOIR_AUDIO_DATA(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), CAINTEOIR_TYPE_AUDIO_DATA, CainteoirAudioData))
#define CAINTEOIR_AUDIO_DATA_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), CAINTEOIR_TYPE_AUDIO_DATA, CainteoirAudioDataClass))
#define CAINTEOIR_IS_AUDIO_DATA(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), CAINTEOIR_TYPE_AUDIO_DATA))
#define CAINTEOIR_IS_AUDIO_DATA_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), CAINTEOIR_TYPE_AUDIO_DATA))
#define CAINTEOIR_AUDIO_DATA_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), CAINTEOIR_TYPE_AUDIO_DATA, CainteoirAudioDataClass))

typedef struct _CainteoirAudioData        CainteoirAudioData;
typedef struct _CainteoirAudioDataPrivate CainteoirAudioDataPrivate;
typedef struct _CainteoirAudioDataClass   CainteoirAudioDataClass;

struct _CainteoirAudioData
{
	GObject parent_instance;

	/*< private >*/
	CainteoirAudioDataPrivate *priv;
};

struct _CainteoirAudioDataClass
{
	GObjectClass parent_class;

	/* Padding for future expansion */
	void (*_reserved1)(void);
	void (*_reserved2)(void);
	void (*_reserved3)(void);
	void (*_reserved4)(void);
};

GType                                  cainteoir_audio_data_get_type(void) G_GNUC_CONST;

CainteoirAudioData *                   cainteoir_audio_data_new(const gchar *filename);

G_END_DECLS

#endif
