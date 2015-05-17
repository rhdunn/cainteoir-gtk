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

namespace rdf = cainteoir::rdf;
namespace rql = cainteoir::rdf::query;

struct _CainteoirMetadataPrivate
{
	std::shared_ptr<rdf::graph> metadata;
	rql::results data;
};

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirMetadata, cainteoir_metadata, G_TYPE_OBJECT)

static void
cainteoir_metadata_finalize(GObject *object)
{
	CainteoirMetadata *metadata = CAINTEOIR_METADATA(object);
	metadata->priv->~CainteoirMetadataPrivate();

	G_OBJECT_CLASS(cainteoir_metadata_parent_class)->finalize(object);
}

static void
cainteoir_metadata_class_init(CainteoirMetadataClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = cainteoir_metadata_finalize;
}

static void
cainteoir_metadata_init(CainteoirMetadata *doc)
{
	void * data = cainteoir_metadata_get_instance_private(doc);
	doc->priv = new (data)CainteoirMetadataPrivate();
}

CainteoirMetadata *
cainteoir_metadata_new(const std::shared_ptr<rdf::graph> &metadata,
                       const rdf::uri &subject)
{
	CainteoirMetadata *self = CAINTEOIR_METADATA(g_object_new(CAINTEOIR_TYPE_METADATA, nullptr));
	self->priv->metadata = metadata;
	self->priv->data = rql::select(*self->priv->metadata, rql::subject == subject);
	return self;
}
