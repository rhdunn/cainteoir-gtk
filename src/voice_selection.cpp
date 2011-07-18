/* Voice Selection View
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
#include <cainteoir/engines.hpp>

namespace tts = cainteoir::tts;

#include "voice_selection.hpp"

VoiceSelectionView::VoiceSelectionView(tts::engines &aEngines)
	: mEngines(&aEngines)
	, parameterView(5, 3, false)
	, apply(_("_Apply"), true)
{
	buttons.add(apply);
	buttons.set_layout(Gtk::BUTTONBOX_START);

	pack_start(header, Gtk::PACK_SHRINK);
	pack_start(parameterView);
	pack_start(buttons, Gtk::PACK_SHRINK);

	set_border_width(6);
	parameterView.set_border_width(4);

	header.set_alignment(0, 0);
	header.set_markup(_("<b>Voice Settings</b>"));

	create_entry(tts::parameter::rate, 0);
	create_entry(tts::parameter::volume, 1);
	create_entry(tts::parameter::pitch, 2);
	create_entry(tts::parameter::pitch_range, 3);

	apply.signal_clicked().connect(sigc::mem_fun(*this, &VoiceSelectionView::apply_settings));
}

void VoiceSelectionView::show()
{
	foreach_iter (item, parameters)
	{
		std::tr1::shared_ptr<tts::parameter> parameter = mEngines->parameter(item->type);

		item->param->set_range(parameter->minimum(), parameter->maximum());
		item->param->set_value(parameter->value());

		item->label->set_markup(parameter->name());
		item->units->set_markup(parameter->units());
	}

	Gtk::VBox::show();
}

void VoiceSelectionView::apply_settings()
{
	foreach_iter (item, parameters)
	{
		mEngines->parameter(item->type)->set_value(item->param->get_value());
	}
}

void VoiceSelectionView::create_entry(tts::parameter::type aParameter, int row)
{
	VoiceParameter item;
	item.type = aParameter;

	item.param = Gtk::manage(new Gtk::HScale());
	item.param->set_increments(1.0, 5.0);
	item.param->set_value_pos(Gtk::POS_RIGHT);
	item.param->set_digits(0);

	item.label = Gtk::manage(new Gtk::Label(""));
	item.label->set_alignment(0, 0.5);

	item.units = Gtk::manage(new Gtk::Label(""));
	item.units->set_alignment(0, 0.5);

	parameterView.attach(*item.label, 0, 1, row, row+1, Gtk::FILL, Gtk::FILL, 4, 4);
	parameterView.attach(*item.param, 1, 2, row, row+1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL, 4, 4);
	parameterView.attach(*item.units, 2, 3, row, row+1, Gtk::FILL, Gtk::FILL, 4, 4);

	parameters.push_back(item);
}
