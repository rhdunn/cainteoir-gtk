/* A GTK+ wrapper around cainteoir RDF metadata results.
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

#include <cainteoir-gtk/cainteoir_metadata.h>

#include "cainteoir_metadata_private.h"
#include "extensions/glib.h"

namespace rdf = cainteoir::rdf;
namespace rql = cainteoir::rdf::query;

struct CainteoirMetadataPrivate
{
	std::shared_ptr<rdf::graph> metadata;
	rql::results data;
};

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirMetadata, cainteoir_metadata, G_TYPE_OBJECT)

#define CAINTEOIR_METADATA_PRIVATE(object) \
	((CainteoirMetadataPrivate *)cainteoir_metadata_get_instance_private(CAINTEOIR_METADATA(object)))

GXT_DEFINE_TYPE_CONSTRUCTION(CainteoirMetadata, cainteoir_metadata, CAINTEOIR_METADATA)

static void
cainteoir_metadata_class_init(CainteoirMetadataClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = cainteoir_metadata_finalize;
}

CainteoirMetadata *
cainteoir_metadata_new(const std::shared_ptr<rdf::graph> &metadata,
                       const rdf::uri &subject)
{
	CainteoirMetadata *self = CAINTEOIR_METADATA(g_object_new(CAINTEOIR_TYPE_METADATA, nullptr));
	CainteoirMetadataPrivate *priv = CAINTEOIR_METADATA_PRIVATE(self);
	priv->metadata = metadata;
	priv->data = rql::select(*priv->metadata, rql::subject == subject);
	return self;
}

gchar *
cainteoir_metadata_get_string(CainteoirMetadata *metadata,
                              const gchar *predicate)
{
	CainteoirMetadataPrivate *priv = CAINTEOIR_METADATA_PRIVATE(metadata);
	rdf::uri selector = rdf::href(predicate);
	for (auto &query : priv->data)
	{
		if (rql::predicate(query) == selector)
		{
			const rdf::uri &object = rql::object(query);
			if (object.empty())
				return g_strdup(rql::value(query).c_str());
			else
			{
				for (auto &data : rql::select(*priv->metadata, rql::subject == object))
				{
					if (rql::predicate(data) == rdf::rdf("value"))
						return g_strdup(rql::value(data).c_str());
				}
			}
		}
	}
	return nullptr;
}

GArray *
cainteoir_metadata_get_strings(CainteoirMetadata *metadata,
                               const gchar *predicate)
{
	CainteoirMetadataPrivate *priv = CAINTEOIR_METADATA_PRIVATE(metadata);
	GArray *items = g_array_new(FALSE, TRUE, sizeof(gchar *));
	rdf::uri selector = rdf::href(predicate);
	for (auto &query : priv->data)
	{
		if (rql::predicate(query) == selector)
		{
			const rdf::uri &object = rql::object(query);
			if (object.empty())
			{
				gchar *value = g_strdup(rql::value(query).c_str());
				g_array_append_val(items, value);
			}
			else
			{
				for (auto &data : rql::select(*priv->metadata, rql::subject == object))
				{
					if (rql::predicate(data) == rdf::rdf("value"))
					{
						gchar *value = g_strdup(rql::value(data).c_str());
						g_array_append_val(items, value);
					}
				}
			}
		}
	}
	return items;
}

gchar *
cainteoir_metadata_get_string_by_context(CainteoirMetadata *metadata,
                                         const gchar *predicate,
                                         const gchar *context_predicate,
                                         const gchar *context_value)
{
	CainteoirMetadataPrivate *priv = CAINTEOIR_METADATA_PRIVATE(metadata);
	rdf::uri selector = rdf::href(predicate);
	rdf::uri context  = rdf::href(context_predicate);
	for (auto &query : priv->data)
	{
		if (rql::predicate(query) == selector)
		{
			const rdf::uri &object = rql::object(query);
			if (!object.empty())
			{
				std::string type;
				std::string value;

				for (auto &data : rql::select(*priv->metadata, rql::subject == object))
				{
					if (rql::predicate(data) == rdf::rdf("value"))
						value = rql::value(data);
					else if (rql::predicate(data) == context)
						type = rql::value(data);
				}

				if (!value.empty() && type == context_value)
					return g_strdup(value.c_str());
			}
		}
	}
	return nullptr;
}

GArray *
cainteoir_metadata_get_strings_by_context(CainteoirMetadata *metadata,
                                          const gchar *predicate,
                                          const gchar *context_predicate,
                                          const gchar *context_value)
{
	CainteoirMetadataPrivate *priv = CAINTEOIR_METADATA_PRIVATE(metadata);
	GArray *items = g_array_new(FALSE, TRUE, sizeof(gchar *));
	rdf::uri selector = rdf::href(predicate);
	rdf::uri context  = rdf::href(context_predicate);
	for (auto &query : priv->data)
	{
		if (rql::predicate(query) == selector)
		{
			const rdf::uri &object = rql::object(query);
			if (!object.empty())
			{
				std::string type;
				std::string value;

				for (auto &data : rql::select(*priv->metadata, rql::subject == object))
				{
					if (rql::predicate(data) == rdf::rdf("value"))
						value = rql::value(data);
					else if (rql::predicate(data) == context)
						type = rql::value(data);
				}

				if (!value.empty() && type == context_value)
				{
					gchar *item = g_strdup(value.c_str());
					g_array_append_val(items, item);
				}
			}
		}
	}
	return items;
}
