/* The Cainteoir Reader settings view.
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
#include "i18n.h"

#include <gtk/gtk.h>

#include "reader_settings_view.h"

struct _ReaderSettingsViewPrivate
{
	CainteoirSettings *settings;
};

G_DEFINE_TYPE_WITH_PRIVATE(ReaderSettingsView, reader_settings_view, GTK_TYPE_BIN)

static void
reader_settings_view_finalize(GObject *object)
{
	ReaderSettingsView *view = READER_SETTINGS_VIEW(object);
	g_object_unref(G_OBJECT(view->priv->settings));

	G_OBJECT_CLASS(reader_settings_view_parent_class)->finalize(object);
}

static void
reader_settings_view_class_init(ReaderSettingsViewClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = reader_settings_view_finalize;
}

static void
reader_settings_view_init(ReaderSettingsView *view)
{
	view->priv = (ReaderSettingsViewPrivate *)reader_settings_view_get_instance_private(view);
}

GtkWidget *
reader_settings_view_new(CainteoirSettings *settings)
{
	ReaderSettingsView *view = READER_SETTINGS_VIEW(g_object_new(READER_TYPE_SETTINGS_VIEW, nullptr));
	view->priv->settings = CAINTEOIR_SETTINGS(g_object_ref(G_OBJECT(settings)));

	GtkWidget *layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_add(GTK_CONTAINER(view), layout);

	return GTK_WIDGET(view);
}
