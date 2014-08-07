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
	uint16_t maximum_height;
	float view_duration;
	uint32_t view_offset;

	GtkAdjustment *hadjustment;
	GtkAdjustment *vadjustment;

	guint hscroll_policy : 1;
	guint vscroll_policy : 1;
};

enum
{
	PROP_0,
	// GtkScrollable interface:
	PROP_HADJUSTMENT,
	PROP_VADJUSTMENT,
	PROP_HSCROLL_POLICY,
	PROP_VSCROLL_POLICY,
};

G_DEFINE_TYPE_WITH_CODE(CainteoirWaveformView, cainteoir_waveform_view, GTK_TYPE_DRAWING_AREA,
                        G_IMPLEMENT_INTERFACE(GTK_TYPE_SCROLLABLE, nullptr))

#define CAINTEOIR_WAVEFORM_VIEW_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE((obj), CAINTEOIR_TYPE_WAVEFORM_VIEW, CainteoirWaveformViewPrivate))

static void
cainteoir_waveform_view_value_changed(GtkAdjustment *adjustment, CainteoirWaveformView *view)
{
	if (adjustment == view->priv->hadjustment)
	{
		uint16_t frequency = cainteoir_audio_data_get_frequency(view->priv->data);
		uint32_t sample_count = cainteoir_audio_data_get_sample_count(view->priv->data);
		float offset = gtk_adjustment_get_value(adjustment);
		view->priv->view_offset = std::min((uint32_t)(offset * frequency), sample_count);

		gtk_widget_queue_draw(GTK_WIDGET(view));
	}
}

static void
cainteoir_waveform_view_set_hadjustment_values(CainteoirWaveformView *view)
{
	if (!view->priv->data) return;

	float duration = cainteoir_audio_data_get_duration(view->priv->data);
	g_object_set(view->priv->hadjustment,
	             "lower", 0.0,
	             "upper", std::max(view->priv->view_duration, duration),
	             "page-size", view->priv->view_duration,
	             "step-increment", 0.001, // 1ms
	             "page-increment", view->priv->view_duration,
	             nullptr);
}

static void
cainteoir_waveform_view_set_hadjustment(CainteoirWaveformView *view, GtkAdjustment *adjustment)
{
	if (adjustment && view->priv->hadjustment == adjustment)
		return;

	if (view->priv->hadjustment != nullptr)
	{
		g_signal_handlers_disconnect_by_func(view->priv->hadjustment, (gpointer)cainteoir_waveform_view_value_changed, view);
		g_object_unref(view->priv->hadjustment);
	}

	if (adjustment == nullptr)
		adjustment = gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

	g_signal_connect(adjustment, "value-changed", G_CALLBACK(cainteoir_waveform_view_value_changed), view);
	view->priv->hadjustment = GTK_ADJUSTMENT(g_object_ref_sink(adjustment));
	cainteoir_waveform_view_set_hadjustment_values(view);

	g_object_notify(G_OBJECT(view), "hadjustment");
}

static void
cainteoir_waveform_view_set_vadjustment(CainteoirWaveformView *view, GtkAdjustment *adjustment)
{
	if (adjustment && view->priv->vadjustment == adjustment)
		return;

	if (view->priv->vadjustment != nullptr)
	{
		g_signal_handlers_disconnect_by_func(view->priv->vadjustment, (gpointer)cainteoir_waveform_view_value_changed, view);
		g_object_unref(view->priv->vadjustment);
	}

	if (adjustment == nullptr)
		adjustment = gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

	g_signal_connect(adjustment, "value-changed", G_CALLBACK(cainteoir_waveform_view_value_changed), view);
	view->priv->vadjustment = GTK_ADJUSTMENT(g_object_ref_sink(adjustment));

	g_object_notify(G_OBJECT(view), "vadjustment");
}

static void
cainteoir_waveform_view_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	CainteoirWaveformView *view = CAINTEOIR_WAVEFORM_VIEW(object);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	// GtkScrollable interface:
	case PROP_HADJUSTMENT:
		cainteoir_waveform_view_set_hadjustment(view, GTK_ADJUSTMENT(g_value_get_object(value)));
		break;
	case PROP_VADJUSTMENT:
		cainteoir_waveform_view_set_vadjustment(view, GTK_ADJUSTMENT(g_value_get_object(value)));
		break;
	case PROP_HSCROLL_POLICY:
		view->priv->hscroll_policy = g_value_get_enum(value);
		gtk_widget_queue_resize(GTK_WIDGET(view));
		break;
	case PROP_VSCROLL_POLICY:
		view->priv->vscroll_policy = g_value_get_enum(value);
		gtk_widget_queue_resize(GTK_WIDGET(view));
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
	// GtkScrollable interface:
	case PROP_HADJUSTMENT:
		g_value_set_object(value, view->priv->hadjustment);
		break;
	case PROP_VADJUSTMENT:
		g_value_set_object(value, view->priv->vadjustment);
		break;
	case PROP_HSCROLL_POLICY:
		g_value_set_enum(value, view->priv->hscroll_policy);
		break;
	case PROP_VSCROLL_POLICY:
		g_value_set_enum(value, view->priv->vscroll_policy);
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

	if (!view->priv->data)
		return FALSE;

	uint16_t frequency = cainteoir_audio_data_get_frequency(view->priv->data);
	const short * samples = cainteoir_audio_data_get_s16_samples(view->priv->data);
	uint32_t sample_count = cainteoir_audio_data_get_sample_count(view->priv->data);

	uint32_t sample_window = view->priv->view_duration * frequency;
	if (sample_window == 0) sample_window = sample_count;

	GtkAllocation allocation;
	gtk_widget_get_allocation(widget, &allocation);

	cairo_set_source_rgb(cr, 0, 0, 1);
	cairo_set_line_width(cr, 1);

	cairo_scale(cr, (float)allocation.width / (sample_window / view->priv->window_size), 0.5);

	samples       += view->priv->view_offset;
	sample_window += view->priv->view_offset;

	int midpoint = allocation.height;
	int waveform_height = std::min(allocation.height, (int)view->priv->maximum_height);
	short current = std::numeric_limits<short>::min();
	for (uint32_t sample = view->priv->view_offset, x = 0; sample != sample_window; ++sample)
	{
		current = std::max(current, (sample < sample_count) ? *samples++ : std::numeric_limits<short>::max());
		if (sample % view->priv->window_size != 0)
			continue;

		float value = (float)current / 32768;

		cairo_move_to(cr, x, midpoint - (value * waveform_height));
		cairo_line_to(cr, x, midpoint + (value * waveform_height));
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

	// GtkScrollable interface:
	g_object_class_override_property(object, PROP_HADJUSTMENT,    "hadjustment");
	g_object_class_override_property(object, PROP_VADJUSTMENT,    "vadjustment");
	g_object_class_override_property(object, PROP_HSCROLL_POLICY, "hscroll-policy");
	g_object_class_override_property(object, PROP_VSCROLL_POLICY, "vscroll-policy");
}

static void
cainteoir_waveform_view_init(CainteoirWaveformView *view)
{
	view->priv = CAINTEOIR_WAVEFORM_VIEW_GET_PRIVATE(view);
	view->priv->data = nullptr;
	view->priv->window_size = 16;
	view->priv->maximum_height = std::numeric_limits<uint16_t>::max();
	view->priv->view_duration = 0;
	view->priv->view_offset = 0;
	view->priv->hadjustment = nullptr;
	view->priv->vadjustment = nullptr;
	view->priv->hscroll_policy = 0;
	view->priv->vscroll_policy = 0;

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

	cainteoir_waveform_view_set_hadjustment_values(view);
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

void
cainteoir_waveform_view_set_maximum_height(CainteoirWaveformView *view, uint16_t maximum_height)
{
	g_return_if_fail(CAINTEOIR_WAVEFORM_VIEW(view));
	g_return_if_fail(maximum_height != 0);

	view->priv->maximum_height = maximum_height;
}

uint16_t
cainteoir_waveform_view_get_maximum_height(CainteoirWaveformView *view)
{
	g_return_val_if_fail(CAINTEOIR_WAVEFORM_VIEW(view), 0);

	return view->priv->maximum_height;
}

void
cainteoir_waveform_view_set_view_duration(CainteoirWaveformView *view, float view_duration)
{
	g_return_if_fail(CAINTEOIR_WAVEFORM_VIEW(view));
	g_return_if_fail(view_duration > 0);

	view->priv->view_duration = view_duration;
}

float
cainteoir_waveform_view_get_view_duration(CainteoirWaveformView *view)
{
	g_return_val_if_fail(CAINTEOIR_WAVEFORM_VIEW(view), 0);

	return view->priv->view_duration;
}
