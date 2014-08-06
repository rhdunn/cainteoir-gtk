/* A GTK+ wrapper around the DocumentFormat/AudioFormat metadata.
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

#include <cainteoir-gtk/cainteoir_supported_formats.h>
#include <cainteoir/document.hpp>
#include <cainteoir/audio.hpp>

namespace rdf = cainteoir::rdf;
namespace rql = cainteoir::rdf::query;

struct _CainteoirSupportedFormatsPrivate
{
	rdf::graph metadata;
	rql::results mimetypes;
	rdf::uri format;
};

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirSupportedFormats, cainteoir_supported_formats, G_TYPE_OBJECT)

static void
cainteoir_supported_formats_finalize(GObject *object)
{
	CainteoirSupportedFormats *doc = CAINTEOIR_SUPPORTED_FORMATS(object);
	doc->priv->~CainteoirSupportedFormatsPrivate();

	G_OBJECT_CLASS(cainteoir_supported_formats_parent_class)->finalize(object);
}

static void
cainteoir_supported_formats_class_init(CainteoirSupportedFormatsClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = cainteoir_supported_formats_finalize;
}

static void
cainteoir_supported_formats_init(CainteoirSupportedFormats *doc)
{
	void * data = cainteoir_supported_formats_get_instance_private(doc);
	doc->priv = new (data)CainteoirSupportedFormatsPrivate();
}

CainteoirSupportedFormats *
cainteoir_supported_formats_new(CainteoirFormatType type)
{
	CainteoirSupportedFormats *formats = CAINTEOIR_SUPPORTED_FORMATS(g_object_new(CAINTEOIR_TYPE_SUPPORTED_FORMATS, nullptr));
	switch (type)
	{
	case CAINTEOIR_DOCUMENT_FORMATS:
		cainteoir::supportedDocumentFormats(formats->priv->metadata, cainteoir::text_support);
		formats->priv->format = rdf::tts("DocumentFormat");
		break;
	case CAINTEOIR_AUDIO_FORMATS:
		cainteoir::supported_audio_formats(formats->priv->metadata);
		formats->priv->format = rdf::tts("AudioFormat");
		break;
	default:
		fprintf(stderr, "error: unknown cainteoir format type\n");
		g_object_unref(formats);
		return nullptr;
	}
	formats->priv->mimetypes = rql::select(formats->priv->metadata, rql::predicate == rdf::tts("mimetype"));
	return formats;
}

GtkRecentFilter *
cainteoir_supported_formats_create_recent_filter(CainteoirSupportedFormats *formats)
{
	GtkRecentFilter *filter = gtk_recent_filter_new();

	rdf::graph &metadata = formats->priv->metadata;
	for (auto &format : rql::select(metadata,
	                                rql::predicate == rdf::rdf("type") &&
	                                rql::object    == formats->priv->format))
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
	rdf::graph &metadata = formats->priv->metadata;
	for (auto &format : rql::select(metadata,
	                                rql::predicate == rdf::rdf("type") &&
	                                rql::object    == formats->priv->format))
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

gboolean cainteoir_supported_formats_is_mimetype_supported(CainteoirSupportedFormats *formats, const gchar *mimetype)
{
	for (auto &mime : formats->priv->mimetypes)
	{
		if (rql::value(mime) == mimetype)
			return TRUE;
	}
	return FALSE;
}
