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

#include "extensions/glib.h"

#include <math.h>

typedef struct _CainteoirTimeBarPrivate CainteoirTimeBarPrivate;

struct _CainteoirTimeBarPrivate
{
	GtkWidget *elapsed;
	GtkWidget *total;
	GtkWidget *progress;
};

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirTimeBar, cainteoir_timebar, GTK_TYPE_BOX)

#define CAINTEOIR_TIMEBAR_PRIVATE(object) \
	((CainteoirTimeBarPrivate *)cainteoir_timebar_get_instance_private(CAINTEOIR_TIMEBAR(object)))

GXT_DEFINE_TYPE_CONSTRUCTION(CainteoirTimeBar, cainteoir_timebar, CAINTEOIR_TIMEBAR)

static void
cainteoir_timebar_class_init(CainteoirTimeBarClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = cainteoir_timebar_finalize;
}

GtkWidget *
cainteoir_timebar_new()
{
	CainteoirTimeBar *self = CAINTEOIR_TIMEBAR(g_object_new(CAINTEOIR_TYPE_TIMEBAR, nullptr));
	CainteoirTimeBarPrivate *priv = CAINTEOIR_TIMEBAR_PRIVATE(self);
	priv->elapsed = gtk_label_new("00:00:00.0");
	priv->total = gtk_label_new("00:00:00.0");
	priv->progress = gtk_progress_bar_new();

	gtk_widget_set_valign(priv->progress, GTK_ALIGN_CENTER);

	gtk_box_pack_start(GTK_BOX(self), priv->elapsed, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(self), priv->progress, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(self), priv->total, FALSE, TRUE, 0);

	return GTK_WIDGET(self);
}

static void
format_time(GtkWidget *time_label, gdouble seconds)
{
	int ms = int(seconds * 10.0) % 10;

	int minutes = floor(seconds / 60.0);
	seconds = seconds - (minutes * 60.0);

	int hours = floor(minutes / 60.0);
	minutes = minutes - (hours * 60.0);

	char time[80];
	snprintf(time, sizeof(time), "%02d:%02d:%02d.%01d", hours, minutes, (int)floor(seconds), ms);
	gtk_label_set_text(GTK_LABEL(time_label), time);
}

void
cainteoir_timebar_set_time(CainteoirTimeBar *timebar,
                           gdouble elapsed_time,
                           gdouble total_time)
{
	CainteoirTimeBarPrivate *priv = CAINTEOIR_TIMEBAR_PRIVATE(timebar);
	format_time(priv->elapsed, elapsed_time);
	format_time(priv->total, total_time);
}

void
cainteoir_timebar_set_progress(CainteoirTimeBar *timebar,
                               gdouble progress)
{
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(CAINTEOIR_TIMEBAR_PRIVATE(timebar)->progress),
	                              progress / 100.0);
}
