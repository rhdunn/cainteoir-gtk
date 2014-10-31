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

#ifndef CAINTEOIR_GTK_CAINTEOIR_AUDIO_DATA_S16_H
#define CAINTEOIR_GTK_CAINTEOIR_AUDIO_DATA_S16_H

#include <stdint.h>

G_BEGIN_DECLS

#define CAINTEOIR_TYPE_AUDIO_DATA_S16 \
	(cainteoir_audio_data_s16_get_type())
#define CAINTEOIR_AUDIO_DATA_S16(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), CAINTEOIR_TYPE_AUDIO_DATA_S16, CainteoirAudioDataS16))
#define CAINTEOIR_AUDIO_DATA_S16_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), CAINTEOIR_TYPE_AUDIO_DATA_S16, CainteoirAudioDataS16Class))
#define CAINTEOIR_IS_AUDIO_DATA_S16(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), CAINTEOIR_TYPE_AUDIO_DATA_S16))
#define CAINTEOIR_IS_AUDIO_DATA_S16_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), CAINTEOIR_TYPE_AUDIO_DATA_S16))
#define CAINTEOIR_AUDIO_DATA_S16_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), CAINTEOIR_TYPE_AUDIO_DATA_S16, CainteoirAudioDataS16Class))

typedef struct _CainteoirAudioDataS16        CainteoirAudioDataS16;
typedef struct _CainteoirAudioDataS16Private CainteoirAudioDataS16Private;
typedef struct _CainteoirAudioDataS16Class   CainteoirAudioDataS16Class;

struct _CainteoirAudioDataS16
{
	GObject parent_instance;

	/*< private >*/
	CainteoirAudioDataS16Private *priv;
};

struct _CainteoirAudioDataS16Class
{
	GObjectClass parent_class;

	/* Padding for future expansion */
	void (*_reserved1)(void);
	void (*_reserved2)(void);
	void (*_reserved3)(void);
	void (*_reserved4)(void);
};

GType                                  cainteoir_audio_data_s16_get_type(void) G_GNUC_CONST;

CainteoirAudioDataS16 *                cainteoir_audio_data_s16_new(const gchar *filename,
                                                                    const char *start_time,
                                                                    const char *end_time,
                                                                    uint16_t channel,
                                                                    uint16_t frequency);

uint16_t                               cainteoir_audio_data_s16_get_frequency(CainteoirAudioDataS16 *audio);

uint32_t                               cainteoir_audio_data_s16_get_sample_count(CainteoirAudioDataS16 *audio);

float                                  cainteoir_audio_data_s16_get_duration(CainteoirAudioDataS16 *audio);

const short *                          cainteoir_audio_data_s16_get_samples(CainteoirAudioDataS16 *audio);

G_END_DECLS

#endif
