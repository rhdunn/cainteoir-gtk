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

static GtkWidget *create_nav_item(const char *name, bool is_button)
{
	GtkWidget *item;
	if (is_button)
	{
		item = gtk_button_new_with_label(name);
	}
	else
	{
		item = gtk_label_new(name);
	}

	GtkToolItem *toolitem = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(toolitem), item);
	return GTK_WIDGET(toolitem);
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

	GtkWidget *view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

	GtkCellRenderer *renderer = cainteoir_library_entry_cell_renderer_new();
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("", renderer,
		"title", LIB_TITLE,
		nullptr);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	gtk_widget_set_name(view, "toc");
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);

	toc_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	gtk_tree_selection_set_mode(toc_selection, GTK_SELECTION_MULTIPLE);

	GtkWidget *scrolled_view = gtk_scrolled_window_new(nullptr, nullptr);
	gtk_container_add(GTK_CONTAINER(scrolled_view), GTK_WIDGET(view));

	GtkWidget *topbar = gtk_toolbar_new();
	gtk_widget_set_name(topbar, "breadcrumb-bar");

	gtk_container_add(GTK_CONTAINER(topbar), create_nav_item(i18n("All"), true));
	gtk_container_add(GTK_CONTAINER(topbar), create_nav_item("\xE2\x9D\xAD", false));
	gtk_container_add(GTK_CONTAINER(topbar), create_nav_item(i18n("Recent"), true));

	GtkToolItem *expander = gtk_tool_item_new();
	gtk_tool_item_set_expand(GTK_TOOL_ITEM(expander), TRUE);
	gtk_container_add(GTK_CONTAINER(topbar), GTK_WIDGET(expander));

	layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(layout), topbar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(layout), scrolled_view, TRUE, TRUE, 0);

	update_recent(aRecent, aMetadata, 30);
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

		for (auto mimetype = mimetypes.begin(), last = mimetypes.end(); mimetype != last; ++mimetype)
		{
			if (rql::value(*mimetype) == mime)
			{
				cainteoir::document_events events;
				char *uri  = gtk_recent_info_get_uri_display(info);
				try
				{
					if (cainteoir::parseDocument(uri, events, metadata))
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
