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

#include "config.h"

#include <gtk/gtk.h>

#include <cainteoir-gtk/cainteoir_supported_formats.h>
#include <cainteoir/document.hpp>
#include <cainteoir/audio.hpp>

#include "extensions/glib.h"

namespace rdf = cainteoir::rdf;
namespace rql = cainteoir::rdf::query;

typedef struct _CainteoirSupportedFormatsPrivate CainteoirSupportedFormatsPrivate;

struct _CainteoirSupportedFormatsPrivate
{
	rdf::graph metadata;
	rql::results mimetypes;
	rdf::uri format;
};

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirSupportedFormats, cainteoir_supported_formats, G_TYPE_OBJECT)

#define CAINTEOIR_SUPPORTED_FORMATS_PRIVATE(object) \
	((CainteoirSupportedFormatsPrivate *)cainteoir_supported_formats_get_instance_private(CAINTEOIR_SUPPORTED_FORMATS(object)))

GXT_DEFINE_TYPE_CONSTRUCTION(CainteoirSupportedFormats, cainteoir_supported_formats, CAINTEOIR_SUPPORTED_FORMATS)

static void
cainteoir_supported_formats_class_init(CainteoirSupportedFormatsClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = cainteoir_supported_formats_finalize;
}

CainteoirSupportedFormats *
cainteoir_supported_formats_new(CainteoirFormatType type)
{
	CainteoirSupportedFormats *formats = CAINTEOIR_SUPPORTED_FORMATS(g_object_new(CAINTEOIR_TYPE_SUPPORTED_FORMATS, nullptr));
	CainteoirSupportedFormatsPrivate *priv = CAINTEOIR_SUPPORTED_FORMATS_PRIVATE(formats);
	switch (type)
	{
	case CAINTEOIR_DOCUMENT_FORMATS:
		cainteoir::supportedDocumentFormats(priv->metadata, cainteoir::text_support);
		priv->format = rdf::tts("DocumentFormat");
		break;
	case CAINTEOIR_AUDIO_FORMATS:
		cainteoir::supported_audio_formats(priv->metadata);
		priv->format = rdf::tts("AudioFormat");
		break;
	case CAINTEOIR_METADATA_FORMATS:
		cainteoir::supportedDocumentFormats(priv->metadata, cainteoir::metadata_support);
		priv->format = rdf::tts("DocumentFormat");
		break;
	default:
		fprintf(stderr, "error: unknown cainteoir format type\n");
		g_object_unref(formats);
		return nullptr;
	}
	priv->mimetypes = rql::select(priv->metadata, rql::predicate == rdf::tts("mimetype"));
	return formats;
}

GtkRecentFilter *
cainteoir_supported_formats_create_recent_filter(CainteoirSupportedFormats *formats)
{
	CainteoirSupportedFormatsPrivate *priv = CAINTEOIR_SUPPORTED_FORMATS_PRIVATE(formats);
	GtkRecentFilter *filter = gtk_recent_filter_new();

	rdf::graph &metadata = priv->metadata;
	for (auto &format : rql::select(metadata,
	                                rql::predicate == rdf::rdf("type") &&
	                                rql::object    == priv->format))
	{
		for (auto &mimetype : rql::select(metadata,
		                                  rql::predicate == rdf::tts("mimetype") &&
		                                  rql::subject   == rql::subject(format)))
			gtk_recent_filter_add_mime_type(filter, rql::value(mimetype).c_str());
	}

	return filter;
}

void
cainteoir_supported_formats_add_file_filters(CainteoirSupportedFormats *formats, GtkFileChooser *chooser, const gchar *active_mimetype)
{
	CainteoirSupportedFormatsPrivate *priv = CAINTEOIR_SUPPORTED_FORMATS_PRIVATE(formats);
	rdf::graph &metadata = priv->metadata;
	for (auto &format : rql::select(metadata,
	                                rql::predicate == rdf::rdf("type") &&
	                                rql::object    == priv->format))
	{
		rql::results data = rql::select(metadata, rql::subject == rql::subject(format));

		GtkFileFilter *filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, rql::select_value<std::string>(data, rql::predicate == rdf::dc("title")).c_str());

		bool active_filter = false;
		for (auto &item : rql::select(data, rql::predicate == rdf::tts("mimetype")))
		{
			const std::string &mimetype = rql::value(item);
			gtk_file_filter_add_mime_type(filter, mimetype.c_str());
			if (mimetype == active_mimetype)
				active_filter = true;
		}

		gtk_file_chooser_add_filter(chooser, filter);
		if (active_filter)
			gtk_file_chooser_set_filter(chooser, filter);
	}
}

gboolean
cainteoir_supported_formats_is_mimetype_supported(CainteoirSupportedFormats *formats, const gchar *mimetype)
{
	for (auto &mime : CAINTEOIR_SUPPORTED_FORMATS_PRIVATE(formats)->mimetypes)
	{
		if (rql::value(mime) == mimetype)
			return TRUE;
	}
	return FALSE;
}

gboolean
cainteoir_supported_formats_file_info(CainteoirSupportedFormats *formats,
                                      const gchar *filename,
                                      gchar **type,
                                      gchar **mimetype)
{
	CainteoirSupportedFormatsPrivate *priv = CAINTEOIR_SUPPORTED_FORMATS_PRIVATE(formats);
	const char *extpos = strrchr(filename, '.');
	if (!extpos)
		return FALSE;

	std::string ext = '*' + std::string(extpos);
	for (auto &filetype : rql::select(priv->metadata, rql::predicate == rdf::tts("extension")))
	{
		if (rql::value(filetype) == ext)
		{
			const rdf::uri &uri = rql::subject(filetype);

			if (type)
			{
				auto match = rql::select_value<std::string>(priv->metadata,
				             rql::subject == uri && rql::predicate == rdf::tts("name"));
				*type = g_strdup(match.c_str());
			}

			if (mimetype)
			{
				auto match = rql::select_value<std::string>(priv->metadata,
				             rql::subject == uri && rql::predicate == rdf::tts("mimetype"));
				*mimetype = g_strdup(match.c_str());
			}

			return TRUE;
		}
	}
	return FALSE;
}
