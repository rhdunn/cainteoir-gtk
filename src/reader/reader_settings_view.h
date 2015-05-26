/* The Cainteoir Reader settings view.
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

#ifndef CAINTEOIR_READER_SETTINGS_VIEW_H
#define CAINTEOIR_READER_SETTINGS_VIEW_H

typedef struct _CainteoirSettings           CainteoirSettings;
typedef struct _CainteoirSpeechSynthesizers CainteoirSpeechSynthesizers;

G_BEGIN_DECLS

#define READER_TYPE_SETTINGS_VIEW \
	(reader_settings_view_get_type())
#define READER_SETTINGS_VIEW(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), READER_TYPE_SETTINGS_VIEW, ReaderSettingsView))
#define READER_SETTINGS_VIEW_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), READER_TYPE_SETTINGS_VIEW, ReaderSettingsViewClass))
#define READER_IS_SETTINGS_VIEW(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), READER_TYPE_SETTINGS_VIEW))
#define READER_IS_SETTINGS_VIEW_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), READER_TYPE_SETTINGS_VIEW))
#define READER_SETTINGS_VIEW_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), READER_TYPE_SETTINGS_VIEW, ReaderSettingsViewClass))

typedef struct _ReaderSettingsView        ReaderSettingsView;
typedef struct _ReaderSettingsViewPrivate ReaderSettingsViewPrivate;
typedef struct _ReaderSettingsViewClass   ReaderSettingsViewClass;

struct _ReaderSettingsView
{
	GtkBin parent_instance;

	/*< private >*/
	ReaderSettingsViewPrivate *priv;
};

struct _ReaderSettingsViewClass
{
	GtkBinClass parent_class;

	/* Padding for future expansion */
	void (*_reserved1)(void);
	void (*_reserved2)(void);
	void (*_reserved3)(void);
	void (*_reserved4)(void);
};

GType                                  reader_settings_view_get_type(void) G_GNUC_CONST;

GtkWidget *                            reader_settings_view_new(CainteoirSettings *settings,
                                                                CainteoirSpeechSynthesizers *synthesizers);

void                                   reader_settings_view_update_speech_parameters(ReaderSettingsView *view);

G_END_DECLS

#endif
