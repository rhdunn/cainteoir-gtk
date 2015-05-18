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

enum IndexColumns
{
	INDEX_ENTRY_PTR,
	INDEX_GUTTER,
	INDEX_TITLE,
	INDEX_ANCHOR,
	INDEX_DISPLAY,
	INDEX_COUNT
};

struct _CainteoirDocumentIndexPrivate
{
	GtkTreeStore *store;
	GtkTreeSelection *selection;
	std::vector<cainteoir::ref_entry> index;
};

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirDocumentIndex, cainteoir_document_index, GTK_TYPE_TREE_VIEW)

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
	index->priv->store = gtk_tree_store_new(INDEX_COUNT, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
}

static void
add_icon_column(GtkTreeView *treeview, int i)
{
	GtkCellRenderer *renderer = gtk_cell_renderer_pixbuf_new();
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("", renderer, "icon-name", i, nullptr);
	gtk_tree_view_append_column(treeview, column);
}

static void
add_text_column(GtkTreeView *treeview, int i)
{
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("", renderer, "text", i, nullptr);
	gtk_tree_view_append_column(treeview, column);
}

GtkWidget *
cainteoir_document_index_new(void)
{
	CainteoirDocumentIndex *self = CAINTEOIR_DOCUMENT_INDEX(g_object_new(CAINTEOIR_TYPE_DOCUMENT_INDEX, nullptr));

	gtk_tree_view_set_model(GTK_TREE_VIEW(self), GTK_TREE_MODEL(self->priv->store));
	add_icon_column(GTK_TREE_VIEW(self), INDEX_GUTTER);
	add_text_column(GTK_TREE_VIEW(self), INDEX_DISPLAY);

	gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(self), TRUE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(self), FALSE);
	gtk_tree_view_set_tooltip_column(GTK_TREE_VIEW(self), INDEX_TITLE);

	self->priv->selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self));
	gtk_tree_selection_set_mode(self->priv->selection, GTK_SELECTION_MULTIPLE);

	return GTK_WIDGET(self);
}

void
cainteoir_document_index_build(CainteoirDocumentIndex *index,
                               CainteoirDocument *doc,
                               const gchar *index_type)
{
	rdf::graph &metadata = *cainteoir_document_get_rdf_metadata(doc);
	rdf::uri &subject = *cainteoir_document_get_subject(doc);
	index->priv->index = cainteoir::navigation(metadata, subject, rdf::href(index_type));

	gtk_tree_store_clear(index->priv->store);

	GtkTreeIter row;
	int initial_depth = index->priv->index.front().depth;
	for (const auto &entry : index->priv->index)
	{
		std::ostringstream title;
		for (const auto &i : cainteoir::range<int>(initial_depth, entry.depth))
			title << "... ";
		title << entry.title;

		gtk_tree_store_append(index->priv->store, &row, nullptr);
		gtk_tree_store_set(index->priv->store, &row,
			INDEX_ENTRY_PTR, &entry,
			INDEX_GUTTER,    "",
			INDEX_TITLE,     entry.title.c_str(),
			INDEX_ANCHOR,    entry.location.str().c_str(),
			INDEX_DISPLAY,   title.str().c_str(),
			-1);
	}
}
