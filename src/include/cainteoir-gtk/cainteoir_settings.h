/* Helper APIs for parsing Cainteoir Settings files.
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

#ifndef CAINTEOIR_GTK_CAINTEOIR_SETTINGS_H
#define CAINTEOIR_GTK_CAINTEOIR_SETTINGS_H

G_BEGIN_DECLS

#define CAINTEOIR_TYPE_SETTINGS \
	(cainteoir_settings_get_type())
#define CAINTEOIR_SETTINGS(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), CAINTEOIR_TYPE_SETTINGS, CainteoirSettings))
#define CAINTEOIR_SETTINGS_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), CAINTEOIR_TYPE_SETTINGS, CainteoirSettingsClass))
#define CAINTEOIR_IS_SETTINGS(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), CAINTEOIR_TYPE_SETTINGS))
#define CAINTEOIR_IS_SETTINGS_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), CAINTEOIR_TYPE_SETTINGS))
#define CAINTEOIR_SETTINGS_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), CAINTEOIR_TYPE_SETTINGS, CainteoirSettingsClass))

typedef struct _CainteoirSettings                CainteoirSettings;
typedef struct _CainteoirSettingsClass           CainteoirSettingsClass;

struct _CainteoirSettings
{
	GObject parent_instance;
};

struct _CainteoirSettingsClass
{
	GObjectClass parent_class;

	/* Padding for future expansion */
	void (*_reserved1)(void);
	void (*_reserved2)(void);
	void (*_reserved3)(void);
	void (*_reserved4)(void);
};

GType                                  cainteoir_settings_get_type(void) G_GNUC_CONST;

CainteoirSettings *                    cainteoir_settings_new(const gchar *filename);

void                                   cainteoir_settings_save(CainteoirSettings *settings);

gchar *                                cainteoir_settings_get_string(CainteoirSettings *settings,
                                                                     const gchar *group,
                                                                     const gchar *key,
                                                                     const gchar *default_value);

void                                   cainteoir_settings_set_string(CainteoirSettings *settings,
                                                                      const gchar *group,
                                                                      const gchar *key,
                                                                      const gchar *value);

gboolean                               cainteoir_settings_get_boolean(CainteoirSettings *settings,
                                                                      const gchar *group,
                                                                      const gchar *key,
                                                                      gboolean default_value);

void                                   cainteoir_settings_set_boolean(CainteoirSettings *settings,
                                                                      const gchar *group,
                                                                      const gchar *key,
                                                                      gboolean value);

gint                                   cainteoir_settings_get_integer(CainteoirSettings *settings,
                                                                      const gchar *group,
                                                                      const gchar *key,
                                                                      gint default_value);

void                                   cainteoir_settings_set_integer(CainteoirSettings *settings,
                                                                      const gchar *group,
                                                                      const gchar *key,
                                                                      gint value);

G_END_DECLS

#endif
