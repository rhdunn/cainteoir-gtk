/* A GTK+ wrapper around the DocumentFormat/AudioFormat metadata.
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

#ifndef CAINTEOIR_GTK_CAINTEOIR_SUPPORTED_FORMATS_H
#define CAINTEOIR_GTK_CAINTEOIR_SUPPORTED_FORMATS_H

G_BEGIN_DECLS

#define CAINTEOIR_TYPE_SUPPORTED_FORMATS \
	(cainteoir_supported_formats_get_type())
#define CAINTEOIR_SUPPORTED_FORMATS(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), CAINTEOIR_TYPE_SUPPORTED_FORMATS, CainteoirSupportedFormats))
#define CAINTEOIR_SUPPORTED_FORMATS_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), CAINTEOIR_TYPE_SUPPORTED_FORMATS, CainteoirSupportedFormatsClass))
#define CAINTEOIR_IS_SUPPORTED_FORMATS(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), CAINTEOIR_TYPE_SUPPORTED_FORMATS))
#define CAINTEOIR_IS_SUPPORTED_FORMATS_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), CAINTEOIR_TYPE_SUPPORTED_FORMATS))
#define CAINTEOIR_SUPPORTED_FORMATS_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), CAINTEOIR_TYPE_SUPPORTED_FORMATS, CainteoirSupportedFormatsClass))

typedef struct _CainteoirSupportedFormats        CainteoirSupportedFormats;
typedef struct _CainteoirSupportedFormatsClass   CainteoirSupportedFormatsClass;

struct _CainteoirSupportedFormats
{
	GObject parent_instance;
};

struct _CainteoirSupportedFormatsClass
{
	GObjectClass parent_class;

	/* Padding for future expansion */
	void (*_reserved1)(void);
	void (*_reserved2)(void);
	void (*_reserved3)(void);
	void (*_reserved4)(void);
};

typedef enum _CainteoirFormatType
{
	CAINTEOIR_DOCUMENT_FORMATS,
	CAINTEOIR_AUDIO_FORMATS,
	CAINTEOIR_METADATA_FORMATS,
} CainteoirFormatType;

GType                                  cainteoir_supported_formats_get_type(void) G_GNUC_CONST;

CainteoirSupportedFormats *            cainteoir_supported_formats_new(CainteoirFormatType type);

GtkRecentFilter *                      cainteoir_supported_formats_create_recent_filter(CainteoirSupportedFormats *formats);

void                                   cainteoir_supported_formats_add_file_filters(CainteoirSupportedFormats *formats,
                                                                                    GtkFileChooser *chooser,
                                                                                    const gchar *active_mimetype);

gboolean                               cainteoir_supported_formats_is_mimetype_supported(CainteoirSupportedFormats *formats,
                                                                                         const gchar *mimetype);

gboolean                               cainteoir_supported_formats_file_info(CainteoirSupportedFormats *formats,
                                                                             const gchar *filename,
                                                                             gchar **type,
                                                                             gchar **mimetype);

G_END_DECLS

#endif
