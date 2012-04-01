/* Time Bar
 *
 * Copyright (C) 2011-2012 Reece H. Dunn
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

#include <math.h>

#include "timebar.hpp"

static void format_time(char *s, int n, double seconds)
{
	int ms = int(seconds * 10.0) % 10;

	int minutes = floor(seconds / 60.0);
	seconds = seconds - (minutes * 60.0);

	int hours = floor(minutes / 60.0);
	minutes = minutes - (hours * 60.0);

	snprintf(s, n, "%02d:%02d:%02d.%01d", hours, minutes, (int)floor(seconds), ms);
}

TimeBar::TimeBar()
{
	progress = gtk_progress_bar_new();
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress), FALSE);
	gtk_widget_set_name(progress, "timebar");

	elapsedTime = gtk_label_new("00:00:00.0");
	totalTime   = gtk_label_new("00:00:00.0");

	GtkWidget *hlayout = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_box_pack_start(GTK_BOX(hlayout), elapsedTime, FALSE, FALSE, 4);
	gtk_box_pack_start(GTK_BOX(hlayout), progress, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hlayout), totalTime, FALSE, FALSE, 4);

	layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(layout), hlayout, FALSE, FALSE, 0);

	gtk_widget_set_valign(layout, GTK_ALIGN_CENTER);
}

void TimeBar::update(double elapsed, double total, double completed)
{
	char elapsed_time[80];
	char total_time[80];

	if (completed < 0.0 || completed > 100.0)
	{
		fprintf(stderr, "completed progress value out of range [0..100], got: %G\n", completed);
		return;
	}

	format_time(elapsed_time, 80, elapsed);
	format_time(total_time, 80, total);

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), completed / 100.0);

	gtk_label_set_text(GTK_LABEL(elapsedTime), elapsed_time);
	gtk_label_set_text(GTK_LABEL(totalTime), total_time);
}
