/* Metadata View
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
#include <gtk/gtk.h>

#include "metadata.hpp"
#include "gtk-compatibility.hpp"

namespace rql = cainteoir::rdf::query;

MetadataView::MetadataView(cainteoir::languages & lang, const char *label, int rows)
	: languages(lang)
{
	layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_set_border_width(GTK_CONTAINER(layout), 6);

	char labelname[512];
	snprintf(labelname, sizeof(labelname), "<b>%s</b>", label);

	header = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(header), 0, 0);
	gtk_label_set_markup(GTK_LABEL(header), labelname);

	metadata = gtk_grid_new();
	gtk_container_set_border_width(GTK_CONTAINER(metadata), 4);
	gtk_grid_set_row_spacing(GTK_GRID(metadata), 4);
	gtk_grid_set_column_spacing(GTK_GRID(metadata), 15);

	gtk_box_pack_start(GTK_BOX(layout), header,   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(layout), metadata, FALSE, FALSE, 0);
}

void MetadataView::clear()
{
	for(auto item = values.begin(), last = values.end(); item != last; ++item)
	{
		gtk_label_set_label(GTK_LABEL(item->second.second), "");
		gtk_widget_hide(item->second.first);
		gtk_widget_hide(item->second.second);
	}
}

void MetadataView::add_metadata(const rdf::graph & aMetadata, const rdf::uri & aUri, const rdf::uri & aPredicate)
{
	rql::results selection = rql::select(aMetadata, rql::matches(rql::subject, aUri));
	for(auto query = selection.begin(), last = selection.end(); query != last; ++query)
	{
		if (rql::predicate(*query).as<rdf::uri>()->ns == rdf::dc || rql::predicate(*query).as<rdf::uri>()->ns == rdf::dcterms)
		{
			rdf::resource object = rql::object(*query);
			if (object.as<rdf::literal>())
			{
				if (rql::predicate(*query).as<rdf::uri>()->ref == aPredicate.ref)
				{
					if (aPredicate == rdf::dc("language"))
						add_metadata(aPredicate, languages(rql::value(object)).c_str());
					else
						add_metadata(aPredicate, rql::value(object).c_str());
				}
			}
			else
			{
				rql::results selection = rql::select(aMetadata, rql::matches(rql::subject, object));

				if (rql::predicate(*query).as<rdf::uri>()->ref == "creator" && aPredicate == rdf::dc("creator"))
				{
					std::string role;
					std::string author;

					for(auto data = selection.begin(), last = selection.end(); data != last; ++data)
					{
						const std::string &object = rql::value(*data);
						if (rql::predicate(*data) == rdf::rdf("value"))
							author = object;
						else if (rql::predicate(*data) == rdf::opf("role") || rql::predicate(*data) == rdf::pkg("role"))
							role = object;
					}

					if (!author.empty() && (role == "aut" || role.empty()))
						add_metadata(aPredicate, author.c_str());
				}
				else if (rql::predicate(*query).as<rdf::uri>()->ref == aPredicate.ref)
				{
					for(auto data = selection.begin(), last = selection.end(); data != last; ++data)
					{
						const std::string &object = rql::value(*data);
						if (rql::predicate(*data) == rdf::rdf("value"))
						{
							if (aPredicate == rdf::dc("language"))
								add_metadata(aPredicate, languages(object).c_str());
							else
								add_metadata(aPredicate, object.c_str());
						}
					}
				}
			}
		}
		else if (rql::predicate(*query) == aPredicate)
			add_metadata(aPredicate, rql::value(*query).c_str());
	}
}

void MetadataView::add_metadata(const rdf::uri & aPredicate, const char * value)
{
	auto &item = values[aPredicate.str()];

	gtk_label_set_label(GTK_LABEL(item.second), value);
	gtk_widget_show(item.first);
	gtk_widget_show(item.second);
}

void MetadataView::create_entry(const rdf::uri & aPredicate, const char * labelText, int row)
{
	GtkWidget *label = gtk_label_new("");
	GtkWidget *value = gtk_label_new("");

	values[aPredicate.str()] = std::make_pair(label, value);

	gtk_widget_set_size_request(label, 80, 0);
	gtk_misc_set_alignment(GTK_MISC(label), 1, 0);
	{
		GtkStyleContext *context = gtk_widget_get_style_context(label);
		gtk_style_context_add_class(context, "label");
		gtk_label_set_label(GTK_LABEL(label), labelText);
	}

	gtk_misc_set_alignment(GTK_MISC(value), 0, 0);
	gtk_label_set_line_wrap(GTK_LABEL(value), true);
	gtk_label_set_width_chars(GTK_LABEL(value), 40);

	gtk_grid_attach(GTK_GRID(metadata), label, 0, row, 1, 1);
	gtk_grid_attach(GTK_GRID(metadata), value, 1, row, 1, 1);
}
