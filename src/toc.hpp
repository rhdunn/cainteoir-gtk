/* Table of Content Side Pane
 *
 * Copyright (C) 2011-2012 Reece H. Dunn
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

#ifndef CAINTEOIRGTK_SRC_TOC_HPP
#define CAINTEOIRGTK_SRC_TOC_HPP

#include <cainteoir/metadata.hpp>

namespace rdf = cainteoir::rdf;

typedef std::pair<const rdf::uri, const rdf::uri> TocSelection;

class TocPane
{
public:
	TocPane();

	operator GtkWidget *() { return layout; }

	void connect(GtkWidget *aTextView);

	bool empty() const;

	void clear();

	void add(int depth, const rdf::uri &location, const std::string &title);

	TocSelection selection() const;
private:
	GtkWidget *layout;
	GtkWidget *view;
	GtkTreeStore *store;
	GtkTreeSelection *toc_selection;
};

#endif
