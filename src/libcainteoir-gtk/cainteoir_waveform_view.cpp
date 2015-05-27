/* Audio Waveform Viewer.
 *
 * Copyright (C) 2014-2015 Reece H. Dunn
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
#include <cainteoir-gtk/cainteoir_audio_data_s16.h>

#include "extensions/glib.h"

#include <algorithm>
#include <cstdlib>
#include <climits>

typedef struct _CainteoirWaveformViewPrivate CainteoirWaveformViewPrivate;

struct _CainteoirWaveformViewPrivate
{
	CainteoirAudioDataS16 *data;
	uint16_t window_size;
	uint16_t maximum_height;
	float view_duration;
	uint32_t view_offset;

	GtkAdjustment *hadjustment;
	GtkAdjustment *vadjustment;

	guint hscroll_policy : 1;
	guint vscroll_policy : 1;

	_CainteoirWaveformViewPrivate()
		: data(nullptr)
		, window_size(16)
		, maximum_height(std::numeric_limits<uint16_t>::max())
		, view_duration(0)
		, view_offset(0)
		, hadjustment(nullptr)
		, vadjustment(nullptr)
		, hscroll_policy(0)
		, vscroll_policy(0)
	{
	}

	~_CainteoirWaveformViewPrivate()
	{
		if (data) g_object_unref(data);
	}
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
                        G_ADD_PRIVATE(CainteoirWaveformView)
                        G_IMPLEMENT_INTERFACE(GTK_TYPE_SCROLLABLE, nullptr))

#define CAINTEOIR_WAVEFORM_VIEW_PRIVATE(object) \
	((CainteoirWaveformViewPrivate *)cainteoir_waveform_view_get_instance_private(CAINTEOIR_WAVEFORM_VIEW(object)))

GXT_DEFINE_TYPE_CONSTRUCTION(CainteoirWaveformView, cainteoir_waveform_view, CAINTEOIR_WAVEFORM_VIEW)

static void
cainteoir_waveform_view_value_changed(GtkAdjustment *adjustment, CainteoirWaveformView *view)
{
	CainteoirWaveformViewPrivate *priv = CAINTEOIR_WAVEFORM_VIEW_PRIVATE(view);
	if (adjustment == priv->hadjustment)
	{
		uint16_t frequency = cainteoir_audio_data_s16_get_frequency(priv->data);
		uint32_t sample_count = cainteoir_audio_data_s16_get_sample_count(priv->data);
		float offset = gtk_adjustment_get_value(adjustment);
		priv->view_offset = std::min((uint32_t)(offset * frequency), sample_count);

		gtk_widget_queue_draw(GTK_WIDGET(view));
	}
}

static void
cainteoir_waveform_view_set_hadjustment_values(CainteoirWaveformView *view)
{
	CainteoirWaveformViewPrivate *priv = CAINTEOIR_WAVEFORM_VIEW_PRIVATE(view);
	if (!priv->data) return;

	float duration = cainteoir_audio_data_s16_get_duration(priv->data);
	g_object_set(priv->hadjustment,
	             "lower", 0.0,
	             "upper", std::max(priv->view_duration, duration),
	             "page-size", priv->view_duration,
	             "step-increment", 0.001, // 1ms
	             "page-increment", priv->view_duration,
	             nullptr);
}

static void
cainteoir_waveform_view_set_hadjustment(CainteoirWaveformView *view, GtkAdjustment *adjustment)
{
	CainteoirWaveformViewPrivate *priv = CAINTEOIR_WAVEFORM_VIEW_PRIVATE(view);
	if (adjustment && priv->hadjustment == adjustment)
		return;

	if (priv->hadjustment != nullptr)
	{
		g_signal_handlers_disconnect_by_func(priv->hadjustment, (gpointer)cainteoir_waveform_view_value_changed, view);
		g_object_unref(priv->hadjustment);
	}

	if (adjustment == nullptr)
		adjustment = gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

	g_signal_connect(adjustment, "value-changed", G_CALLBACK(cainteoir_waveform_view_value_changed), view);
	priv->hadjustment = GTK_ADJUSTMENT(g_object_ref_sink(adjustment));
	cainteoir_waveform_view_set_hadjustment_values(view);

	g_object_notify(G_OBJECT(view), "hadjustment");
}

static void
cainteoir_waveform_view_set_vadjustment(CainteoirWaveformView *view, GtkAdjustment *adjustment)
{
	CainteoirWaveformViewPrivate *priv = CAINTEOIR_WAVEFORM_VIEW_PRIVATE(view);
	if (adjustment && priv->vadjustment == adjustment)
		return;

	if (priv->vadjustment != nullptr)
	{
		g_signal_handlers_disconnect_by_func(priv->vadjustment, (gpointer)cainteoir_waveform_view_value_changed, view);
		g_object_unref(priv->vadjustment);
	}

	if (adjustment == nullptr)
		adjustment = gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

	g_signal_connect(adjustment, "value-changed", G_CALLBACK(cainteoir_waveform_view_value_changed), view);
	priv->vadjustment = GTK_ADJUSTMENT(g_object_ref_sink(adjustment));

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
		CAINTEOIR_WAVEFORM_VIEW_PRIVATE(view)->hscroll_policy = g_value_get_enum(value);
		gtk_widget_queue_resize(GTK_WIDGET(view));
		break;
	case PROP_VSCROLL_POLICY:
		CAINTEOIR_WAVEFORM_VIEW_PRIVATE(view)->vscroll_policy = g_value_get_enum(value);
		gtk_widget_queue_resize(GTK_WIDGET(view));
		break;
	}
}

static void
cainteoir_waveform_view_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	CainteoirWaveformViewPrivate *priv = CAINTEOIR_WAVEFORM_VIEW_PRIVATE(object);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	// GtkScrollable interface:
	case PROP_HADJUSTMENT:
		g_value_set_object(value, priv->hadjustment);
		break;
	case PROP_VADJUSTMENT:
		g_value_set_object(value, priv->vadjustment);
		break;
	case PROP_HSCROLL_POLICY:
		g_value_set_enum(value, priv->hscroll_policy);
		break;
	case PROP_VSCROLL_POLICY:
		g_value_set_enum(value, priv->vscroll_policy);
		break;
	}
}

static gboolean
cainteoir_waveform_view_draw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	CainteoirWaveformViewPrivate *priv = CAINTEOIR_WAVEFORM_VIEW_PRIVATE(widget);

	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_paint(cr);

	if (!priv->data)
		return FALSE;

	uint16_t frequency = cainteoir_audio_data_s16_get_frequency(priv->data);
	const short * samples = cainteoir_audio_data_s16_get_samples(priv->data);
	uint32_t sample_count = cainteoir_audio_data_s16_get_sample_count(priv->data);

	uint32_t sample_window = priv->view_duration * frequency;
	if (sample_window == 0) sample_window = sample_count;

	GtkAllocation allocation;
	gtk_widget_get_allocation(widget, &allocation);

	cairo_set_source_rgb(cr, 0, 0, 1);
	cairo_set_line_width(cr, 1);

	cairo_scale(cr, (float)allocation.width / (sample_window / priv->window_size), 0.5);

	samples      += priv->view_offset;
	sample_window = std::min(sample_window + priv->view_offset, sample_count);

	int midpoint = allocation.height;
	int waveform_height = std::min(allocation.height, (int)priv->maximum_height);
	short upper = std::numeric_limits<short>::min();
	short lower = std::numeric_limits<short>::max();
	for (uint32_t sample = priv->view_offset, x = 0; sample != sample_window; ++sample)
	{
		upper = std::max(upper, *samples);
		lower = std::min(lower, *samples);
		++samples;
		if (sample % priv->window_size != 0)
			continue;

		cairo_move_to(cr, x, midpoint - ((float)std::abs(upper) / 32768 * waveform_height));
		cairo_line_to(cr, x, midpoint + ((float)std::abs(lower) / 32768 * waveform_height));
		cairo_stroke(cr);

		upper = std::numeric_limits<short>::min();
		lower = std::numeric_limits<short>::max();
		++x;
	}

	return TRUE;
}

static void
cainteoir_waveform_view_class_init(CainteoirWaveformViewClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->set_property = cainteoir_waveform_view_set_property;
	object->get_property = cainteoir_waveform_view_get_property;
	object->finalize = cainteoir_waveform_view_finalize;

	// GtkScrollable interface:
	g_object_class_override_property(object, PROP_HADJUSTMENT,    "hadjustment");
	g_object_class_override_property(object, PROP_VADJUSTMENT,    "vadjustment");
	g_object_class_override_property(object, PROP_HSCROLL_POLICY, "hscroll-policy");
	g_object_class_override_property(object, PROP_VSCROLL_POLICY, "vscroll-policy");
}

GtkWidget *
cainteoir_waveform_view_new()
{
	CainteoirWaveformView *self = CAINTEOIR_WAVEFORM_VIEW(g_object_new(CAINTEOIR_TYPE_WAVEFORM_VIEW, nullptr));
	g_signal_connect(G_OBJECT(self), "draw", G_CALLBACK(cainteoir_waveform_view_draw), nullptr);
	return GTK_WIDGET(self);
}

void
cainteoir_waveform_view_set_s16_data(CainteoirWaveformView *view, CainteoirAudioDataS16 *data)
{
	CainteoirWaveformViewPrivate *priv = CAINTEOIR_WAVEFORM_VIEW_PRIVATE(view);

	if (priv->data) g_object_unref(priv->data);
	priv->data = CAINTEOIR_AUDIO_DATA_S16(g_object_ref(data));

	cainteoir_waveform_view_set_hadjustment_values(view);
}

CainteoirAudioDataS16 *
cainteoir_waveform_view_get_s16_data(CainteoirWaveformView *view)
{
	CainteoirWaveformViewPrivate *priv = CAINTEOIR_WAVEFORM_VIEW_PRIVATE(view);
	return CAINTEOIR_AUDIO_DATA_S16(g_object_ref(priv->data));
}

void
cainteoir_waveform_view_set_window_size(CainteoirWaveformView *view, uint16_t window_size)
{
	g_return_if_fail(window_size != 0);

	CAINTEOIR_WAVEFORM_VIEW_PRIVATE(view)->window_size = window_size;
}

uint16_t
cainteoir_waveform_view_get_window_size(CainteoirWaveformView *view)
{
	return CAINTEOIR_WAVEFORM_VIEW_PRIVATE(view)->window_size;
}

void
cainteoir_waveform_view_set_maximum_height(CainteoirWaveformView *view, uint16_t maximum_height)
{
	g_return_if_fail(maximum_height != 0);

	CAINTEOIR_WAVEFORM_VIEW_PRIVATE(view)->maximum_height = maximum_height;
}

uint16_t
cainteoir_waveform_view_get_maximum_height(CainteoirWaveformView *view)
{
	return CAINTEOIR_WAVEFORM_VIEW_PRIVATE(view)->maximum_height;
}

void
cainteoir_waveform_view_set_view_duration(CainteoirWaveformView *view, float view_duration)
{
	g_return_if_fail(view_duration > 0);

	CAINTEOIR_WAVEFORM_VIEW_PRIVATE(view)->view_duration = view_duration;
}

float
cainteoir_waveform_view_get_view_duration(CainteoirWaveformView *view)
{
	return CAINTEOIR_WAVEFORM_VIEW_PRIVATE(view)->view_duration;
}
