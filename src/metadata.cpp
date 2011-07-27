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
#include <gtkmm.h>
#include <cainteoir/platform.hpp>

#include "metadata.hpp"

namespace rql = cainteoir::rdf::query;

MetadataView::MetadataView(cainteoir::languages & lang, const char *label, int rows)
	: metadata(rows, 2, false)
	, languages(lang)
{
	pack_start(header, Gtk::PACK_SHRINK);
	pack_start(metadata, Gtk::PACK_SHRINK);

	set_border_width(6);
	metadata.set_border_width(4);

	header.set_alignment(0, 0);
	header.set_markup(label);
}

void MetadataView::clear()
{
	for(auto item = values.begin(), last = values.end(); item != last; ++item)
		item->second.second->set_label("");
}

void MetadataView::add_metadata(const rdf::graph & aMetadata, const rdf::uri & aUri, const rdf::uri & aPredicate)
{
	rql::results selection = rql::select(aMetadata, rql::matches(rql::subject, aUri));
	for(auto query = selection.begin(), last = selection.end(); query != last; ++query)
	{
		if (rql::predicate(*query).as<rdf::uri>()->ns == rdf::dc || rql::predicate(*query).as<rdf::uri>()->ns == rdf::dcterms)
		{
			rdf::any_type object = rql::object(*query);
			if (object.as<rdf::literal>())
			{
				if (rql::predicate(*query).as<rdf::uri>()->ref == aPredicate.ref)
				{
					if (aPredicate == rdf::dc("language"))
						values[aPredicate.str()].second->set_label(languages(rql::value(object)));
					else
						values[aPredicate.str()].second->set_label(rql::value(object));
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
						values[aPredicate.str()].second->set_label(author);
				}
				else if (rql::predicate(*query).as<rdf::uri>()->ref == aPredicate.ref)
				{
					for(auto data = selection.begin(), last = selection.end(); data != last; ++data)
					{
						const std::string &object = rql::value(*data);
						if (rql::predicate(*data) == rdf::rdf("value"))
						{
							if (aPredicate == rdf::dc("language"))
								values[aPredicate.str()].second->set_label(languages(object));
							else
								values[aPredicate.str()].second->set_label(object);
						}
					}
				}
			}
		}
	}
}

void MetadataView::add_metadata(const rdf::uri & aPredicate, const char * value)
{
	values[aPredicate.str()].second->set_label(value);
}

void MetadataView::create_entry(const rdf::uri & aPredicate, const char * labelText, int row)
{
	Gtk::Label * label = Gtk::manage(new Gtk::Label());
	Gtk::Label * value = Gtk::manage(new Gtk::Label());

	values[aPredicate.str()] = std::make_pair(label, value);

	label->set_alignment(0, 0);
	label->set_markup(labelText);

	value->set_alignment(0, 0);
	value->set_line_wrap(true);

	metadata.attach(*label, 0, 1, row, row+1, Gtk::FILL, Gtk::FILL, 4, 4);
	metadata.attach(*value, 1, 2, row, row+1, Gtk::FILL, Gtk::FILL, 4, 4);
}
