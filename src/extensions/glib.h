/* GLib Extensions.
 *
 * Copyright (C) 2015 Reece H. Dunn
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

#ifndef CAINTEOIR_GTK_EXTENSIONS_GLIB_H
#define CAINTEOIR_GTK_EXTENSIONS_GLIB_H

#include <new>

#define GXT_DEFINE_TYPE_CONSTRUCTION(OBJECT_CLASS, OBJECT_FN_PREFIX, OBJECT_TYPE) \
	static void \
	OBJECT_FN_PREFIX##_finalize(GObject *object) \
	{ \
		OBJECT_CLASS##Private *priv = OBJECT_TYPE##_PRIVATE(object); \
		(priv)->~OBJECT_CLASS##Private(); \
		\
		G_OBJECT_CLASS(OBJECT_FN_PREFIX##_parent_class)->finalize(object); \
	} \
	\
	static void \
	OBJECT_FN_PREFIX##_init(OBJECT_CLASS *self) \
	{ \
		OBJECT_CLASS##Private *priv = OBJECT_TYPE##_PRIVATE(self); \
		new (priv)OBJECT_CLASS##Private(); \
	}

#endif
