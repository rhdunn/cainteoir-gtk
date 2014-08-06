/* Display a cainteoir_document object.
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

#include <cainteoir-gtk/cainteoir_document_view.h>

struct _CainteoirDocumentViewPrivate
{
};

enum
{
	PROP_0,
};

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirDocumentView, cainteoir_document_view, GTK_TYPE_BIN)

static void
cainteoir_document_view_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	CainteoirDocumentView *view = CAINTEOIR_DOCUMENT_VIEW(object);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
cainteoir_document_view_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	CainteoirDocumentView *view = CAINTEOIR_DOCUMENT_VIEW(object);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
cainteoir_document_view_finalize(GObject *object)
{
	CainteoirDocumentView *view = CAINTEOIR_DOCUMENT_VIEW(object);

	G_OBJECT_CLASS(cainteoir_document_view_parent_class)->finalize(object);
}

static void
cainteoir_document_view_class_init(CainteoirDocumentViewClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->set_property = cainteoir_document_view_set_property;
	object->get_property = cainteoir_document_view_get_property;
	object->finalize = cainteoir_document_view_finalize;
}

static void
cainteoir_document_view_init(CainteoirDocumentView *view)
{
	view->priv = (CainteoirDocumentViewPrivate *)cainteoir_document_view_get_instance_private(view);
}

GtkWidget *
cainteoir_document_view_new()
{
	return (GtkWidget *)g_object_new(CAINTEOIR_TYPE_DOCUMENT_VIEW, nullptr);
}
