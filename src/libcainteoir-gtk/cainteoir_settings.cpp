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

#include "config.h"

#include <gtk/gtk.h>

#include <cainteoir-gtk/cainteoir_settings.h>

#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>

struct _CainteoirSettingsPrivate
{
	GKeyFile *settings;
	gchar *filename;
};

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirSettings, cainteoir_settings, G_TYPE_OBJECT)

static void
cainteoir_settings_finalize(GObject *object)
{
	CainteoirSettings *doc = CAINTEOIR_SETTINGS(object);
	if (doc->priv->settings) g_key_file_free(doc->priv->settings);
	if (doc->priv->filename) g_free(doc->priv->filename);

	G_OBJECT_CLASS(cainteoir_settings_parent_class)->finalize(object);
}

static void
cainteoir_settings_class_init(CainteoirSettingsClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = cainteoir_settings_finalize;
}

static void
cainteoir_settings_init(CainteoirSettings *doc)
{
	doc->priv = (CainteoirSettingsPrivate *)cainteoir_settings_get_instance_private(doc);
	doc->priv->settings = g_key_file_new();
	doc->priv->filename = nullptr;
}

CainteoirSettings *
cainteoir_settings_new(const gchar *filename)
{
	CainteoirSettings *settings = CAINTEOIR_SETTINGS(g_object_new(CAINTEOIR_TYPE_SETTINGS, nullptr));

	gchar *root = g_strdup_printf("%s/.cainteoir", getenv("HOME"));
	mkdir(root, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IROTH);
	settings->priv->filename = g_strdup_printf("%s/%s", root, filename ? filename : "settings.dat");
	g_free(root);

	return settings;
}
