/* Table of Content Side Pane
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

#include "toc.hpp"

TocPane::TocPane()
{
	data = Gtk::ListStore::create(model);
	set_model(data);
	append_column(_("Contents"), model.title);

	get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
	set_rubber_banding();
}

void TocPane::clear()
{
	data->clear();
}

void TocPane::add(int depth, const rdf::uri &location, const std::string &title)
{
	Gtk::TreeModel::Row row = *data->append();
	row[model.title] = title;
	row[model.location] = location;
}

TocSelection TocPane::selection() const
{
	std::vector<Gtk::TreePath> selected = get_selection()->get_selected_rows();
	switch (selected.size())
	{
	case 0:
		return TocSelection(rdf::uri(), rdf::uri());
	case 1:
		return TocSelection((*data->get_iter(selected.front()))[model.location],
		                    rdf::uri());
	default:
		{
			auto end = data->get_iter(selected.back());
			++end;
			if (end == data->children().end())
				return TocSelection((*data->get_iter(selected.front()))[model.location],
				                    rdf::uri());
			else
				return TocSelection((*data->get_iter(selected.front()))[model.location],
				                    (*end)[model.location]);
		}
	}
}
