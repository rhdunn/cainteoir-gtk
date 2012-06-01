/* Language and System compatibility helpers.
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

#ifndef CAINTEOIRGTK_SRC_COMPATIBILITY_HPP
#define CAINTEOIRGTK_SRC_COMPATIBILITY_HPP

#include <gtk/gtk.h>

#if !GTK_CHECK_VERSION(3, 3, 0)
void gtk_window_set_hide_titlebar_when_maximized(GtkWindow *, gboolean);
#endif

#ifndef HAVE_CPP_NULLPTR
#	include <stddef.h>

	const struct nullptr_t 
	{
		template <typename T>
		inline operator T *() const { return 0; }

		template <typename C, typename T>
		inline operator T C:: *() const { return 0; }
	private:
		void operator&() const;
	} nullptr = {};

#ifndef HAVE_CPP_IMPLICIT_NULLPTR_COMPARE
	template <typename T>
	inline bool operator==(const T &a, const nullptr_t &b)
	{
		return a == NULL;
	}

	template <typename T>
	inline bool operator==(const nullptr_t &a, const T &b)
	{
		return b == NULL;
	}

	template <typename T>
	inline bool operator!=(const T &a, const nullptr_t &b)
	{
		return a != NULL;
	}

	template <typename T>
	inline bool operator!=(const nullptr_t &a, const T &b)
	{
		return b != NULL;
	}
#endif
#endif


#endif
