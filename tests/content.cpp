/* Document Content Rendering Model tests.
 *
 * Copyright (C) 2012 Reece H. Dunn
 *
 * This file is part of cainteoir-engine.
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

#include <gtk/gtk.h>

#include "document.hpp"
#include "tester.hpp"

REGISTER_TESTSUITE("content");

static void test_tag_(GtkTextTag *aTag, const char *aName,
                      bool aFontFamilySet, const char *aFontFamily,
                      bool aFontSizeSet, int aFontSize,
                      bool aRiseSet, int aRise,
                      bool aJustificationSet, GtkJustification aJustification,
                      bool aStrikeThroughSet, bool aStrikeThrough,
                      bool aUnderlineSet, PangoUnderline aUnderline,
                      bool aStyleSet, PangoStyle aStyle,
                      bool aVariantSet, PangoVariant aVariant,
                      bool aWeightSet, PangoWeight aWeight,
                      const char *location, int line)
{
	gchar *name = nullptr;
	gboolean rise_set = FALSE;
	gint rise = 0;
	gboolean justification_set = FALSE;
	GtkJustification justification = GTK_JUSTIFY_LEFT;
	gboolean strikethrough_set = FALSE;
	gboolean strikethrough = FALSE;
	gboolean underline_set = FALSE;
	PangoUnderline underline = PANGO_UNDERLINE_SINGLE;
	gboolean style_set = FALSE;
	PangoStyle style = PANGO_STYLE_NORMAL;
	gboolean variant_set = FALSE;
	PangoVariant variant = PANGO_VARIANT_NORMAL;
	gboolean weight_set = FALSE;
	PangoWeight weight = PANGO_WEIGHT_NORMAL;
	gboolean font_family_set = FALSE;
	gchar *font_family = nullptr;
	gboolean font_size_set = FALSE;
	gdouble font_size = 0;

	g_object_get(G_OBJECT(aTag),
		"name", &name,
		"rise-set", &rise_set, "rise", &rise,
		"justification-set", &justification_set, "justification", &justification,
		"strikethrough-set", &strikethrough_set, "strikethrough", &strikethrough,
		"underline-set", &underline_set, "underline", &underline,
		"style-set", &style_set, "style", &style,
		"variant-set", &variant_set, "variant", &variant,
		"weight-set", &weight_set, "weight", &weight,
		"family-set", &font_family_set, "family", &font_family,
		"size-set", &font_size_set, "size-points", &font_size,
		NULL);

	assert_location(strcmp(name, aName) == 0, location, line);
	assert_location(rise_set == aRiseSet, location, line);
	assert_location(rise == aRise, location, line);
	assert_location(justification_set == aJustificationSet, location, line);
	assert_location(justification == aJustification, location, line);
	assert_location(strikethrough_set == aStrikeThroughSet, location, line);
	assert_location(strikethrough == aStrikeThrough, location, line);
	assert_location(underline_set == aUnderlineSet, location, line);
	assert_location(underline == aUnderline, location, line);
	assert_location(style_set == aStyleSet, location, line);
	assert_location(style == aStyle, location, line);
	assert_location(variant_set == aVariantSet, location, line);
	assert_location(variant == aVariant, location, line);
	assert_location(weight_set == aWeightSet, location, line);
	assert_location(weight == aWeight, location, line);
	assert_location(font_family_set == aFontFamilySet, location, line);
	if (font_family && aFontFamily)
		assert_location(strcmp(font_family, aFontFamily) == 0, location, line);
	else
	{
		assert_location(font_family == nullptr, location, line);
		assert_location(aFontFamily == nullptr, location, line);
	}
	assert_location(font_size_set == aFontSizeSet, location, line);
	assert_location(font_size == aFontSize, location, line);

	g_free(font_family);
	g_free(name);
	g_object_unref(aTag);
}

#define test_tag(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t) \
	test_tag_(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, __FILE__, __LINE__)

TEST_CASE("cainteoir::styles.display")
{
	g_type_init();

	cainteoir::styles style;

	style.display = cainteoir::display::inherit;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);

	style.display = cainteoir::display::block;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);

	style.display = cainteoir::display::inlined;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);

	style.display = cainteoir::display::none;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);
}

TEST_CASE("cainteoir::styles.vertical_align")
{
	g_type_init();

	cainteoir::styles style;

	style.vertical_align = cainteoir::vertical_align::inherit;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);

	style.vertical_align = cainteoir::vertical_align::baseline;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		true,  0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);

	style.vertical_align = cainteoir::vertical_align::sub;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		true,  -4096,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);

	style.vertical_align = cainteoir::vertical_align::super;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		true,  4096,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);
}

TEST_CASE("cainteoir::styles.text_align")
{
	g_type_init();

	cainteoir::styles style;

	style.text_align = cainteoir::text_align::inherit;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);

	style.text_align = cainteoir::text_align::left;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		true,  GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);

	style.text_align = cainteoir::text_align::right;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		true,  GTK_JUSTIFY_RIGHT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);

	style.text_align = cainteoir::text_align::center;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		true,  GTK_JUSTIFY_CENTER,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);

	style.text_align = cainteoir::text_align::justify;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		true,  GTK_JUSTIFY_FILL,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);
}

TEST_CASE("cainteoir::styles.text_decoration")
{
	g_type_init();

	cainteoir::styles style;

	style.text_decoration = cainteoir::text_decoration::inherit;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);

	style.text_decoration = cainteoir::text_decoration::none;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		true,  false,
		true,  PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);

	style.text_decoration = cainteoir::text_decoration::underline;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		true,  PANGO_UNDERLINE_SINGLE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);

	style.text_decoration = cainteoir::text_decoration::line_through;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		true,  true,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);
}

TEST_CASE("cainteoir::styles.font_style")
{
	g_type_init();

	cainteoir::styles style;

	style.font_style = cainteoir::font_style::inherit;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);

	style.font_style = cainteoir::font_style::normal;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		true,  PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);

	style.font_style = cainteoir::font_style::italic;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		true,  PANGO_STYLE_ITALIC,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);

	style.font_style = cainteoir::font_style::oblique;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		true,  PANGO_STYLE_OBLIQUE,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);
}

TEST_CASE("cainteoir::styles.font_variant")
{
	g_type_init();

	cainteoir::styles style;

	style.font_variant = cainteoir::font_variant::inherit;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);

	style.font_variant = cainteoir::font_variant::normal;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		true,  PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);

	style.font_variant = cainteoir::font_variant::small_caps;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		true,  PANGO_VARIANT_SMALL_CAPS,
		false, PANGO_WEIGHT_NORMAL);
}

TEST_CASE("cainteoir::styles.font_weight")
{
	g_type_init();

	cainteoir::styles style;

	style.font_weight = cainteoir::font_weight::inherit;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false, PANGO_WEIGHT_NORMAL);

	style.font_weight = cainteoir::font_weight::normal;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		true,  PANGO_WEIGHT_NORMAL);

	style.font_weight = cainteoir::font_weight::bold;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		true,  PANGO_WEIGHT_BOLD);
}

TEST_CASE("cainteoir::styles.font_family")
{
	g_type_init();

	cainteoir::styles style;

	style.font_family = std::string();
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false,  PANGO_WEIGHT_NORMAL);

	style.font_family = "serif";
	test_tag(create_text_tag_from_style("test", style), "test",
		true,  "serif",
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false,  PANGO_WEIGHT_NORMAL);

	style.font_family = "Tahoma";
	test_tag(create_text_tag_from_style("test", style), "test",
		true,  "Tahoma",
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false,  PANGO_WEIGHT_NORMAL);
}

TEST_CASE("cainteoir::styles.font_size")
{
	g_type_init();

	cainteoir::styles style;

	style.font_size = 0;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		false, 0,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false,  PANGO_WEIGHT_NORMAL);

	style.font_size = 10;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		true,  10,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false,  PANGO_WEIGHT_NORMAL);

	style.font_size = 12;
	test_tag(create_text_tag_from_style("test", style), "test",
		false, NULL,
		true,  12,
		false, 0,
		false, GTK_JUSTIFY_LEFT,
		false, false,
		false, PANGO_UNDERLINE_NONE,
		false, PANGO_STYLE_NORMAL,
		false, PANGO_VARIANT_NORMAL,
		false,  PANGO_WEIGHT_NORMAL);
}
