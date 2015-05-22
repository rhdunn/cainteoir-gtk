/* Display a timebar.
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

#include <cainteoir-gtk/cainteoir_timebar.h>

struct _CainteoirTimeBarPrivate
{
};

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirTimeBar, cainteoir_timebar, GTK_TYPE_BOX)

static void
cainteoir_timebar_finalize(GObject *object)
{
	CainteoirTimeBar *timebar = CAINTEOIR_TIMEBAR(object);

	G_OBJECT_CLASS(cainteoir_timebar_parent_class)->finalize(object);
}

static void
cainteoir_timebar_class_init(CainteoirTimeBarClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = cainteoir_timebar_finalize;
}

static void
cainteoir_timebar_init(CainteoirTimeBar *timebar)
{
	timebar->priv = (CainteoirTimeBarPrivate *)cainteoir_timebar_get_instance_private(timebar);
}

GtkWidget *
cainteoir_timebar_new()
{
	CainteoirTimeBar *self = CAINTEOIR_TIMEBAR(g_object_new(CAINTEOIR_TYPE_TIMEBAR, nullptr));

	return GTK_WIDGET(self);
}
