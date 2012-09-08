/* Document Library View
 *
 * Copyright (C) 2012 Reece H. Dunn
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
#include "compatibility.hpp"
#include "i18n.h"

#include "library.hpp"
#include "libraryentrycellrenderer.h"

enum LibColumns
{
	LIB_TITLE,
	LIB_ANCHOR, // rdf::uri
	LIB_COUNT // number of columns
};

namespace rql = cainteoir::rdf::query;

static gint sort_recent_items_mru(GtkRecentInfo *a, GtkRecentInfo *b, gpointer unused)
{
	g_assert(a != NULL && b != NULL);
	return gtk_recent_info_get_modified(b) - gtk_recent_info_get_modified(a);
}

static void add_document(GtkTreeStore *store, rdf::graph &metadata, rdf::uri subject, int pos)
{
	rql::results data = rql::select(metadata, rql::matches(rql::subject, subject));
	std::string title = rql::select_value<std::string>(data, rql::matches(rql::predicate, rdf::dc("title")));
	if (title.empty())
	{
		title = subject.str();
		title = title.substr(title.rfind('/') + 1);
	}

	GtkTreeIter row;
	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
		LIB_TITLE,  title.c_str(),
		LIB_ANCHOR, subject.str().c_str(),
		-1);
}

DocumentLibrary::DocumentLibrary(cainteoir::languages &aLanguages, GtkRecentManager *aRecent, rdf::graph &aMetadata)
{
	store = gtk_tree_store_new(LIB_COUNT, G_TYPE_STRING, G_TYPE_STRING);
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	lib_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));

	GtkCellRenderer *renderer = cainteoir_library_entry_cell_renderer_new();
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("", renderer,
		"title", LIB_TITLE,
		nullptr);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	gtk_widget_set_name(view, "library");
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);

	update_recent(aRecent, aMetadata, 45);
}

void DocumentLibrary::update_recent(GtkRecentManager *aRecent, rdf::graph &aMetadata, int max_items_to_show)
{
	GList *items = g_list_sort(gtk_recent_manager_get_items(aRecent), (GCompareFunc)sort_recent_items_mru);

	rql::results mimetypes = rql::select(aMetadata, rql::matches(rql::predicate, rdf::tts("mimetype")));

	int index = 0;
	for (GList *item = g_list_first(items); item && index != max_items_to_show; item = g_list_next(item))
	{
		GtkRecentInfo *info = (GtkRecentInfo *)item->data;

		const char *uri  = gtk_recent_info_get_uri(info);
		if (strncmp(uri, "file://", 7) != 0)
			continue;

		const char *mime = gtk_recent_info_get_mime_type(info);

		for (auto &mimetype : mimetypes)
		{
			if (rql::value(mimetype) == mime)
			{
				char *uri  = gtk_recent_info_get_uri_display(info);
				try
				{
					auto reader = cainteoir::createDocumentReader(uri, metadata, std::string());
					if (reader)
						add_document(store, metadata, rdf::uri(uri, std::string()), index++);
				}
				catch (const std::exception &)
				{
				}
				g_free(uri);
				break;
			}
		}
	}

	g_list_foreach(items, (GFunc)gtk_recent_info_unref, nullptr);
	g_list_free(items);
}

std::string DocumentLibrary::get_filename() const
{
	std::string filename;

	GList *selected = gtk_tree_selection_get_selected_rows(lib_selection, nullptr);
	if (g_list_length(selected) == 1)
	{
		GtkTreeModel *model = GTK_TREE_MODEL(store);

		GList *item = g_list_first(selected);
		GtkTreeIter iter;
		if (item && gtk_tree_model_get_iter(model, &iter, (GtkTreePath *)item->data))
		{
			gchar *anchor = nullptr;
			gtk_tree_model_get(model, &iter, LIB_ANCHOR, &anchor, -1);

			filename = anchor;
			g_free(anchor);
		}
	}

	g_list_foreach(selected, (GFunc)gtk_tree_path_free, nullptr);
	g_list_free(selected);

	return filename;
}
