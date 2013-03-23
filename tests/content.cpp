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
#include <cainteoir-gtk/document.hpp>
#include "tester.hpp"

REGISTER_TESTSUITE("content");

namespace css = cainteoir::css;

static void test_tag_(GtkTextTag *aTag, const char *aName,
                      bool aFontFamilySet, const char *aFontFamily,
                      bool aFontSizeSet, int aFontSize,
                      bool aMarginLeftSet, int aMarginLeft,
                      bool aMarginTopSet, int aMarginTop,
                      bool aMarginRightSet, int aMarginRight,
                      bool aMarginBottomSet, int aMarginBottom,
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
	gboolean margin_left_set = FALSE;
	gint margin_left = 0;
	gboolean margin_top_set = FALSE;
	gint margin_top = 0;
	gboolean margin_right_set = FALSE;
	gint margin_right = 0;
	gboolean margin_bottom_set = FALSE;
	gint margin_bottom = 0;
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
		"pixels-above-lines-set", &margin_top_set, "pixels-above-lines", &margin_top,
		"pixels-below-lines-set", &margin_bottom_set, "pixels-below-lines", &margin_bottom,
		"left-margin-set", &margin_left_set, "left-margin", &margin_left,
		"right-margin-set", &margin_right_set, "right-margin", &margin_right,
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
	assert_location(margin_left_set == aMarginLeftSet, location, line);
	assert_location(margin_left == aMarginLeft, location, line);
	assert_location(margin_top_set == aMarginTopSet, location, line);
	assert_location(margin_top == aMarginTop, location, line);
	assert_location(margin_right_set == aMarginRightSet, location, line);
	assert_location(margin_right == aMarginRight, location, line);
	assert_location(margin_bottom_set == aMarginBottomSet, location, line);
	assert_location(margin_bottom == aMarginBottom, location, line);
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

#define test_tag(aa, ab, ac, ad, ae, af, ag, ah, ai, aj, ak, al, am, an, ao, ap, aq, ar, as, at, au, av , aw, ax, ay, az, ba, bb) \
	test_tag_(aa, ab, ac, ad, ae, af, ag, ah, ai, aj, ak, al, am, an, ao, ap, aq, ar, as, at, au, av, aw, ax, ay, az, ba, bb, __FILE__, __LINE__)

TEST_CASE("css::styles -- defaults")
{
	g_type_init();

	css::styles style("defaults");
	test_tag(create_text_tag_from_style(style), "defaults",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight
}

TEST_CASE("css::styles.display")
{
	g_type_init();

	css::styles style("display");

	style.display = css::display::inherit;
	test_tag(create_text_tag_from_style(style), "display",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.display = css::display::block;
	test_tag(create_text_tag_from_style(style), "display",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.display = css::display::inlined;
	test_tag(create_text_tag_from_style(style), "display",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.display = css::display::list_item;
	test_tag(create_text_tag_from_style(style), "display",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.display = css::display::table;
	test_tag(create_text_tag_from_style(style), "display",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.display = css::display::table_row;
	test_tag(create_text_tag_from_style(style), "display",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.display = css::display::table_cell;
	test_tag(create_text_tag_from_style(style), "display",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.display = css::display::none;
	test_tag(create_text_tag_from_style(style), "display",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight
}

TEST_CASE("css::styles.vertical_align")
{
	g_type_init();

	css::styles style("vertical-align");

	style.vertical_align = css::vertical_align::baseline;
	test_tag(create_text_tag_from_style(style), "vertical-align",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		true,  0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.vertical_align = css::vertical_align::sub;
	test_tag(create_text_tag_from_style(style), "vertical-align",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		true,  -4096,                    // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.vertical_align = css::vertical_align::super;
	test_tag(create_text_tag_from_style(style), "vertical-align",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		true,  4096,                     // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight
}

TEST_CASE("css::styles.text_align")
{
	g_type_init();

	css::styles style("text-align");

	style.text_align = css::text_align::left;
	test_tag(create_text_tag_from_style(style), "text-align",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		true,  GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.text_align = css::text_align::right;
	test_tag(create_text_tag_from_style(style), "text-align",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		true,  GTK_JUSTIFY_RIGHT,        // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.text_align = css::text_align::center;
	test_tag(create_text_tag_from_style(style), "text-align",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		true,  GTK_JUSTIFY_CENTER,       // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.text_align = css::text_align::justify;
	test_tag(create_text_tag_from_style(style), "text-align",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		true,  GTK_JUSTIFY_FILL,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight
}

TEST_CASE("css::styles.text_decoration")
{
	g_type_init();

	css::styles style("text-decoration");

	style.text_decoration = css::text_decoration::none;
	test_tag(create_text_tag_from_style(style), "text-decoration",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		true,  false,                    // strike-through
		true,  PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.text_decoration = css::text_decoration::underline;
	test_tag(create_text_tag_from_style(style), "text-decoration",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		true,  PANGO_UNDERLINE_SINGLE,   // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.text_decoration = css::text_decoration::line_through;
	test_tag(create_text_tag_from_style(style), "text-decoration",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		true,  true,                     // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight
}

TEST_CASE("css::styles.font_style")
{
	g_type_init();

	css::styles style("font-style");

	style.font_style = css::font_style::normal;
	test_tag(create_text_tag_from_style(style), "font-style",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		true,  PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.font_style = css::font_style::italic;
	test_tag(create_text_tag_from_style(style), "font-style",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		true,  PANGO_STYLE_ITALIC,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.font_style = css::font_style::oblique;
	test_tag(create_text_tag_from_style(style), "font-style",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		true,  PANGO_STYLE_OBLIQUE,      // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight
}

TEST_CASE("css::styles.font_variant_caps")
{
	g_type_init();

	css::styles style("font-variant");

	style.font_variant_caps = css::font_variant_caps::normal;
	test_tag(create_text_tag_from_style(style), "font-variant",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		true,  PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.font_variant_caps = css::font_variant_caps::small_caps;
	test_tag(create_text_tag_from_style(style), "font-variant",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		true,  PANGO_VARIANT_SMALL_CAPS, // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight
}

TEST_CASE("css::styles.font_weight")
{
	g_type_init();

	css::styles style("font-weight");

	style.font_weight = css::font_weight::normal;
	test_tag(create_text_tag_from_style(style), "font-weight",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		true,  PANGO_WEIGHT_NORMAL);     // font-weight

	style.font_weight = css::font_weight::bold;
	test_tag(create_text_tag_from_style(style), "font-weight",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		true,  PANGO_WEIGHT_BOLD);       // font-weight
}

TEST_CASE("css::styles.font_family")
{
	g_type_init();

	css::styles style("font-family");

	style.font_family = "serif";
	test_tag(create_text_tag_from_style(style), "font-family",
		true,  "serif",                  // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.font_family = "Tahoma";
	test_tag(create_text_tag_from_style(style), "font-family",
		true,  "Tahoma",                 // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight
}

TEST_CASE("css::styles.font_size")
{
	g_type_init();

	css::styles style("font-size");

	style.font_size = { 10, css::length::points };
	test_tag(create_text_tag_from_style(style), "font-size",
		false, NULL,                     // font-family
		true,  10,                       // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.font_size = { 2, css::length::picas };
	test_tag(create_text_tag_from_style(style), "font-size",
		false, NULL,                     // font-family
		true,  24,                       // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight
}

TEST_CASE("css::styles.margin.left")
{
	g_type_init();

	css::styles style("margin-left");

	style.margin.left = { 10, css::length::pixels };
	test_tag(create_text_tag_from_style(style), "margin-left",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		true,  10,                       // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.margin.left = { 1, css::length::inches };
	test_tag(create_text_tag_from_style(style), "margin-left",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		true,  96,                       // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight
}

TEST_CASE("css::styles.margin.top")
{
	g_type_init();

	css::styles style("margin-top");

	style.margin.top = { 10, css::length::pixels };
	test_tag(create_text_tag_from_style(style), "margin-top",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		true,  10,                       // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.margin.top = { 1, css::length::inches };
	test_tag(create_text_tag_from_style(style), "margin-top",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		true,  96,                       // margin-top
		false, 0,                        // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight
}

TEST_CASE("css::styles.margin.right")
{
	g_type_init();

	css::styles style("margin-right");

	style.margin.right = { 10, css::length::pixels };
	test_tag(create_text_tag_from_style(style), "margin-right",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		true,  10,                       // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.margin.right = { 1, css::length::inches };
	test_tag(create_text_tag_from_style(style), "margin-right",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		true,  96,                       // margin-right
		false, 0,                        // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight
}

TEST_CASE("css::styles.margin.bottom")
{
	g_type_init();

	css::styles style("margin-bottom");

	style.margin.bottom = { 10, css::length::pixels };
	test_tag(create_text_tag_from_style(style), "margin-bottom",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		true,  10,                       // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight

	style.margin.bottom = { 1, css::length::inches };
	test_tag(create_text_tag_from_style(style), "margin-bottom",
		false, NULL,                     // font-family
		false, 0,                        // font-size
		false, 0,                        // margin-left
		false, 0,                        // margin-top
		false, 0,                        // margin-right
		true,  96,                       // margin-bottom
		false, 0,                        // rise
		false, GTK_JUSTIFY_LEFT,         // justification
		false, false,                    // strike-through
		false, PANGO_UNDERLINE_NONE,     // underline
		false, PANGO_STYLE_NORMAL,       // font-style
		false, PANGO_VARIANT_NORMAL,     // font-variant
		false, PANGO_WEIGHT_NORMAL);     // font-weight
}
