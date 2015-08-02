/* The Cainteoir Reader preferences dialog.
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

#ifndef CAINTEOIR_READER_READER_PREFERENCES_H
#define CAINTEOIR_READER_READER_PREFERENCES_H

typedef struct _CainteoirSettings      CainteoirSettings;
typedef struct _ReaderWindow           ReaderWindow;

G_BEGIN_DECLS

#define READER_TYPE_PREFERENCES \
	(reader_preferences_get_type())
#define READER_PREFERENCES(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), READER_TYPE_PREFERENCES, ReaderPreferences))
#define READER_PREFERENCES_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), READER_TYPE_PREFERENCES, ReaderPreferencesClass))
#define READER_IS_PREFERENCES(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), READER_TYPE_PREFERENCES))
#define READER_IS_PREFERENCES_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), READER_TYPE_PREFERENCES))
#define READER_PREFERENCES_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), READER_TYPE_PREFERENCES, ReaderPreferencesClass))

typedef struct _ReaderPreferences      ReaderPreferences;
typedef struct _ReaderPreferencesClass ReaderPreferencesClass;

struct _ReaderPreferences
{
	GtkDialog parent_instance;
};

struct _ReaderPreferencesClass
{
	GtkDialogClass parent_class;

	/* Padding for future expansion */
	void (*_reserved1)(void);
	void (*_reserved2)(void);
	void (*_reserved3)(void);
	void (*_reserved4)(void);
};

GType                                  reader_preferences_get_type(void) G_GNUC_CONST;

GtkWidget *                            reader_preferences_new(ReaderWindow *window,
                                                              CainteoirSettings *settings);

G_END_DECLS

#endif
