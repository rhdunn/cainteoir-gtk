/* Time Bar
 *
 * Copyright (C) 2011 Reece H. Dunn
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

#include <config.h>
#include <gtkmm.h>

#include "timebar.hpp"
#include "gtk-compatibility.hpp"

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
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress), TRUE);

	elapsedTime = gtk_label_new("00:00:00.0");
	totalTime   = gtk_label_new("00:00:00.0");

	GtkWidget *hlayout = gtk_hbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(hlayout), elapsedTime, FALSE, FALSE, 4);
	gtk_box_pack_start(GTK_BOX(hlayout), progress, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hlayout), totalTime, FALSE, FALSE, 4);

	layout = gtk_vbox_new(TRUE, 0);
	gtk_box_pack_start(GTK_BOX(layout), hlayout, FALSE, FALSE, 0);
}

void TimeBar::update(double elapsed, double total, double completed)
{
	char percentage[20];
	char elapsed_time[80];
	char total_time[80];

	sprintf(percentage, "%0.2f%%", completed);
	format_time(elapsed_time, 80, elapsed);
	format_time(total_time, 80, total);

	double fraction = completed / 100.0;
	if (fraction < 0.005)
	{
		// Do not show the progress bar on percentages below 0.5%. This is due
		// to display issues when trying to show fractions below this value.
		fraction = 0.0;
	}

	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), percentage);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), fraction);

	gtk_label_set_text(GTK_LABEL(elapsedTime), elapsed_time);
	gtk_label_set_text(GTK_LABEL(totalTime), total_time);
}
