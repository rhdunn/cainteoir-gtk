/* Audio Waveform Viewer.
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

#include <cainteoir-gtk/cainteoir_waveform_view.h>

#include <algorithm>
#include <cstdlib>
#include <climits>

struct _CainteoirWaveformViewPrivate
{
	CainteoirAudioData *data;
	uint16_t window_size;
};

enum
{
	PROP_0,
};

G_DEFINE_TYPE(CainteoirWaveformView, cainteoir_waveform_view, GTK_TYPE_DRAWING_AREA)

#define CAINTEOIR_WAVEFORM_VIEW_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE((obj), CAINTEOIR_TYPE_WAVEFORM_VIEW, CainteoirWaveformViewPrivate))

static void
cainteoir_waveform_view_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	CainteoirWaveformView *view = CAINTEOIR_WAVEFORM_VIEW(object);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
cainteoir_waveform_view_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	CainteoirWaveformView *view = CAINTEOIR_WAVEFORM_VIEW(object);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static gboolean
cainteoir_waveform_view_draw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	g_return_val_if_fail(CAINTEOIR_WAVEFORM_VIEW(widget), FALSE);
	CainteoirWaveformView *view = CAINTEOIR_WAVEFORM_VIEW(widget);

	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_paint(cr);

	uint16_t frequency = cainteoir_audio_data_get_frequency(view->priv->data);
	const short * samples = cainteoir_audio_data_get_s16_samples(view->priv->data);
	uint32_t sample_count = cainteoir_audio_data_get_sample_count(view->priv->data);

	GtkAllocation allocation;
	gtk_widget_get_allocation(widget, &allocation);

	cairo_set_source_rgb(cr, 0, 0, 1);
	cairo_set_line_width(cr, 1);

	cairo_scale(cr, (float)allocation.width / (sample_count / view->priv->window_size), 0.5);

	int midpoint = allocation.height;
	short current = std::numeric_limits<short>::min();
	for (uint32_t sample = 0, x = 0; sample != sample_count; ++sample)
	{
		current = std::max(current, *samples++);
		if (sample % view->priv->window_size != 0)
			continue;

		float value = (float)current / 32768;

		cairo_move_to(cr, x, midpoint - (value * midpoint));
		cairo_line_to(cr, x, midpoint + (value * midpoint));
		cairo_stroke(cr);

		current = std::numeric_limits<short>::min();
		++x;
	}

	return TRUE;
}

static void
cainteoir_waveform_view_finalize(GObject *object)
{
	CainteoirWaveformView *view = CAINTEOIR_WAVEFORM_VIEW(object);
	if (view->priv->data) g_object_unref(view->priv->data);

	G_OBJECT_CLASS(cainteoir_waveform_view_parent_class)->finalize(object);
}

static void
cainteoir_waveform_view_class_init(CainteoirWaveformViewClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->set_property = cainteoir_waveform_view_set_property;
	object->get_property = cainteoir_waveform_view_get_property;
	object->finalize = cainteoir_waveform_view_finalize;

	g_type_class_add_private(object, sizeof(CainteoirWaveformViewPrivate));
}

static void
cainteoir_waveform_view_init(CainteoirWaveformView *view)
{
	view->priv = CAINTEOIR_WAVEFORM_VIEW_GET_PRIVATE(view);
	view->priv->data = nullptr;
	view->priv->window_size = 16;

	g_signal_connect(G_OBJECT(view), "draw", G_CALLBACK(cainteoir_waveform_view_draw), nullptr);
}

GtkWidget *
cainteoir_waveform_view_new()
{
	return (GtkWidget *)g_object_new(CAINTEOIR_TYPE_WAVEFORM_VIEW, nullptr);
}

void
cainteoir_waveform_view_set_data(CainteoirWaveformView *view, CainteoirAudioData *data)
{
	g_return_if_fail(CAINTEOIR_WAVEFORM_VIEW(view));

	if (view->priv->data) g_object_unref(view->priv->data);
	view->priv->data = CAINTEOIR_AUDIO_DATA(g_object_ref(data));
}

CainteoirAudioData *
cainteoir_waveform_view_get_data(CainteoirWaveformView *view)
{
	g_return_val_if_fail(CAINTEOIR_WAVEFORM_VIEW(view), nullptr);

	return CAINTEOIR_AUDIO_DATA(g_object_ref(view->priv->data));
}

void
cainteoir_waveform_view_set_window_size(CainteoirWaveformView *view, uint16_t window_size)
{
	g_return_if_fail(CAINTEOIR_WAVEFORM_VIEW(view));
	g_return_if_fail(window_size != 0);

	view->priv->window_size = window_size;
}

uint16_t
cainteoir_waveform_view_get_window_size(CainteoirWaveformView *view)
{
	g_return_val_if_fail(CAINTEOIR_WAVEFORM_VIEW(view), 0);

	return view->priv->window_size;
}
