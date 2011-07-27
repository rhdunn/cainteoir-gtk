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

#ifndef CAINTEOIRGTK_SRC_METADATA_HPP
#define CAINTEOIRGTK_SRC_METADATA_HPP

#include <map>
#include <cainteoir/languages.hpp>
#include <cainteoir/metadata.hpp>

namespace rdf = cainteoir::rdf;

class MetadataView : public Gtk::VBox
{
public:
	MetadataView(cainteoir::languages & lang, const char *label, int rows);

	void clear();

	void add_metadata(const rdf::graph & aMetadata, const rdf::uri & aUri, const rdf::uri & aPredicate);

	void add_metadata(const rdf::uri & aPredicate, const char * value);

	void create_entry(const rdf::uri & aPredicate, const char * labelText, int row);

private:
	Gtk::Label header;
	Gtk::Table metadata;
	std::map<std::string, std::pair<Gtk::Label *, Gtk::Label *> > values;
	cainteoir::languages & languages;
};

#endif
