/* Gtkmm 2 and 3 compatibility helper.
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

#ifndef CAINTEOIRGTK_SRC_GTKOBJECTREF_HPP
#define CAINTEOIRGTK_SRC_GTKOBJECTREF_HPP

/** @brief Gtkmm 2 and 3 compatibility helper.
  *
  * In Gtkmm 2 you need to declare some objects on the stack:
  * @begincode
  *    Gtk::FileFilter filter;
  * @endcode
  * while in Gtkmm 3 you need to use Glib::RefPtr:
  * @begincode
  *    Glib::RefPtr<Gtk::FileFilter> filter;
  * @endcode
  *
  * The GtkObjectRef class is designed to support both of these usages cleanly,
  * so all you need to do is write:
  * @begincode
  *    GtkObjectRef<Gtk::FileFilter> filter;
  * @endcode
  * and use it as you would a Glib::RefPtr.
  */
template <typename T>
struct GtkObjectRef
	#if GTK_MAJOR_VERSION >= 3
		: public Glib::RefPtr<T>
	#else
		: public T
	#endif
{
	GtkObjectRef()
	#if GTK_MAJOR_VERSION >= 3
		: Glib::RefPtr<T>(T::create())
	#endif
	{
	}

	T * operator->()
	{
		#if GTK_MAJOR_VERSION >= 3
			return Glib::RefPtr<T>::operator->();
		#else
			return &*this;
		#endif
	}
};

#endif
