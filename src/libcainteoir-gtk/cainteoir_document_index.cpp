/* A GTK+ wrapper around the cainteoir::ref_entry class.
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

#include <cainteoir-gtk/cainteoir_document_index.h>
#include <cainteoir/document.hpp>

#include "cainteoir_document_private.h"

namespace rdf = cainteoir::rdf;

struct _CainteoirDocumentIndexPrivate
{
	std::vector<cainteoir::ref_entry> index;
};

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirDocumentIndex, cainteoir_document_index, G_TYPE_OBJECT)

static void
cainteoir_document_index_finalize(GObject *object)
{
	CainteoirDocumentIndex *index = CAINTEOIR_DOCUMENT_INDEX(object);
	index->priv->~CainteoirDocumentIndexPrivate();

	G_OBJECT_CLASS(cainteoir_document_index_parent_class)->finalize(object);
}

static void
cainteoir_document_index_class_init(CainteoirDocumentIndexClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = cainteoir_document_index_finalize;
}

static void
cainteoir_document_index_init(CainteoirDocumentIndex *index)
{
	void * data = cainteoir_document_index_get_instance_private(index);
	index->priv = new (data)CainteoirDocumentIndexPrivate();
}

CainteoirDocumentIndex *
cainteoir_document_index_new(void)
{
	CainteoirDocumentIndex *self = CAINTEOIR_DOCUMENT_INDEX(g_object_new(CAINTEOIR_TYPE_DOCUMENT_INDEX, nullptr));
	return self;
}

void
cainteoir_document_index_build(CainteoirDocumentIndex *index,
                               CainteoirDocument *doc,
                               const gchar *index_type)
{
	rdf::graph &metadata = *cainteoir_document_get_rdf_metadata(doc);
	rdf::uri &subject = *cainteoir_document_get_subject(doc);
	index->priv->index = cainteoir::navigation(metadata, subject, rdf::href(index_type));
}
