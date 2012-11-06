/* Document Processing
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
#include "document.hpp"

GtkTextTag *create_text_tag_from_style(const cainteoir::styles &aStyles)
{
	GtkTextTag *tag = gtk_text_tag_new(aStyles.name.c_str());

	switch (aStyles.vertical_align)
	{
	case cainteoir::vertical_align::inherit:
		break;
	case cainteoir::vertical_align::baseline:
		g_object_set(G_OBJECT(tag), "rise-set", TRUE, "rise", 0, NULL);
		break;
	case cainteoir::vertical_align::sub:
		g_object_set(G_OBJECT(tag), "rise-set", TRUE, "rise", -4096, NULL);
		break;
	case cainteoir::vertical_align::super:
		g_object_set(G_OBJECT(tag), "rise-set", TRUE, "rise", 4096, NULL);
		break;
	}

	switch (aStyles.text_align)
	{
	case cainteoir::text_align::inherit:
		break;
	case cainteoir::text_align::left:
		g_object_set(G_OBJECT(tag), "justification-set", TRUE, "justification", GTK_JUSTIFY_LEFT, NULL);
		break;
	case cainteoir::text_align::right:
		g_object_set(G_OBJECT(tag), "justification-set", TRUE, "justification", GTK_JUSTIFY_RIGHT, NULL);
		break;
	case cainteoir::text_align::center:
		g_object_set(G_OBJECT(tag), "justification-set", TRUE, "justification", GTK_JUSTIFY_CENTER, NULL);
		break;
	case cainteoir::text_align::justify:
		g_object_set(G_OBJECT(tag), "justification-set", TRUE, "justification", GTK_JUSTIFY_FILL, NULL);
		break;
	}

	switch (aStyles.text_decoration)
	{
	case cainteoir::text_decoration::inherit:
		break;
	case cainteoir::text_decoration::none:
		g_object_set(G_OBJECT(tag), "strikethrough-set", TRUE, "strikethrough", FALSE, NULL);
		g_object_set(G_OBJECT(tag), "underline-set", TRUE, "underline", PANGO_UNDERLINE_NONE, NULL);
		break;
	case cainteoir::text_decoration::underline:
		g_object_set(G_OBJECT(tag), "underline-set", TRUE, "underline", PANGO_UNDERLINE_SINGLE, NULL);
		break;
	case cainteoir::text_decoration::line_through:
		g_object_set(G_OBJECT(tag), "strikethrough-set", TRUE, "strikethrough", TRUE, NULL);
		break;
	}

	switch (aStyles.font_style)
	{
	case cainteoir::font_style::inherit:
		break;
	case cainteoir::font_style::normal:
		g_object_set(G_OBJECT(tag), "style-set", TRUE, "style", PANGO_STYLE_NORMAL, NULL);
		break;
	case cainteoir::font_style::italic:
		g_object_set(G_OBJECT(tag), "style-set", TRUE, "style", PANGO_STYLE_ITALIC, NULL);
		break;
	case cainteoir::font_style::oblique:
		g_object_set(G_OBJECT(tag), "style-set", TRUE, "style", PANGO_STYLE_OBLIQUE, NULL);
		break;
	}

	switch (aStyles.font_variant)
	{
	case cainteoir::font_variant::inherit:
		break;
	case cainteoir::font_variant::normal:
		g_object_set(G_OBJECT(tag), "variant-set", TRUE, "variant", PANGO_VARIANT_NORMAL, NULL);
		break;
	case cainteoir::font_variant::small_caps:
		g_object_set(G_OBJECT(tag), "variant-set", TRUE, "variant", PANGO_VARIANT_SMALL_CAPS, NULL);
		break;
	}

	switch (aStyles.font_weight)
	{
	case cainteoir::font_weight::inherit:
		break;
	case cainteoir::font_weight::normal:
		g_object_set(G_OBJECT(tag), "weight-set", TRUE, "weight", PANGO_WEIGHT_NORMAL, NULL);
		break;
	case cainteoir::font_weight::bold:
		g_object_set(G_OBJECT(tag), "weight-set", TRUE, "weight", PANGO_WEIGHT_BOLD, NULL);
		break;
	}

	if (!aStyles.font_family.empty())
		g_object_set(G_OBJECT(tag), "family-set", TRUE, "family", aStyles.font_family.c_str(), NULL);

	if (aStyles.font_size != 0)
		g_object_set(G_OBJECT(tag), "size-set", TRUE, "size-points", gdouble(aStyles.font_size), NULL);

	return tag;
}
