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
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct _CainteoirSettingsPrivate CainteoirSettingsPrivate;

struct _CainteoirSettingsPrivate
{
	GKeyFile *settings;
	gchar *filename;
};

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirSettings, cainteoir_settings, G_TYPE_OBJECT)

#define CAINTEOIR_SETTINGS_PRIVATE(object) \
	((CainteoirSettingsPrivate *)cainteoir_settings_get_instance_private(CAINTEOIR_SETTINGS(object)))

static void
cainteoir_settings_finalize(GObject *object)
{
	CainteoirSettingsPrivate *priv = CAINTEOIR_SETTINGS_PRIVATE(object);
	if (priv->settings) g_key_file_free(priv->settings);
	if (priv->filename) g_free(priv->filename);

	G_OBJECT_CLASS(cainteoir_settings_parent_class)->finalize(object);
}

static void
cainteoir_settings_class_init(CainteoirSettingsClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = cainteoir_settings_finalize;
}

static void
cainteoir_settings_init(CainteoirSettings *settings)
{
	CainteoirSettingsPrivate *priv = CAINTEOIR_SETTINGS_PRIVATE(settings);
	priv->settings = g_key_file_new();
	priv->filename = nullptr;
}

CainteoirSettings *
cainteoir_settings_new(const gchar *filename)
{
	CainteoirSettings *settings = CAINTEOIR_SETTINGS(g_object_new(CAINTEOIR_TYPE_SETTINGS, nullptr));
	CainteoirSettingsPrivate *priv = CAINTEOIR_SETTINGS_PRIVATE(settings);

	gchar *root = g_strdup_printf("%s/.cainteoir", getenv("HOME"));
	mkdir(root, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IROTH);
	priv->filename = g_strdup_printf("%s/%s", root, filename ? filename : "settings.dat");
	g_free(root);

	if (!g_key_file_load_from_file(priv->settings, priv->filename, G_KEY_FILE_NONE, nullptr))
	{
		// On error try parsing the old settings format ...

		std::ifstream is(priv->filename);
		std::string group;
		std::string key;
		std::string value;
		while (std::getline(is, group, '.') && std::getline(is, key, '=') && std::getline(is, value))
		{
			if (key == "tts_fallback") key = "tts-fallback";
			g_key_file_set_value(priv->settings, group.c_str(), key.c_str(), value.c_str());
		}
	}

	return settings;
}

void
cainteoir_settings_save(CainteoirSettings *settings)
{
	CainteoirSettingsPrivate *priv = CAINTEOIR_SETTINGS_PRIVATE(settings);
	gsize length = 0;
	gchar *data = g_key_file_to_data(priv->settings, &length, nullptr);
	if (data)
	{
		FILE *out = fopen(priv->filename, "wb");
		if (out)
		{
			fwrite(data, 1, length, out);
			fclose(out);
		}
		g_free(data);
	}
}

gchar *
cainteoir_settings_get_string(CainteoirSettings *settings,
                              const gchar *group,
                              const gchar *key,
                              const gchar *default_value)
{
	GError *error = nullptr;
	gchar *ret = g_key_file_get_string(CAINTEOIR_SETTINGS_PRIVATE(settings)->settings, group, key, &error);
	if (error)
	{
		g_error_free(error);
		return g_strdup(default_value);
	}
	return ret;
}

void
cainteoir_settings_set_string(CainteoirSettings *settings,
                              const gchar *group,
                              const gchar *key,
                              const gchar *value)
{
	g_key_file_set_string(CAINTEOIR_SETTINGS_PRIVATE(settings)->settings, group, key, value);
}

gboolean
cainteoir_settings_get_boolean(CainteoirSettings *settings,
                               const gchar *group,
                               const gchar *key,
                               gboolean default_value)
{
	GError *error = nullptr;
	gboolean ret = g_key_file_get_boolean(CAINTEOIR_SETTINGS_PRIVATE(settings)->settings, group, key, &error);
	if (error)
	{
		g_error_free(error);
		return default_value;
	}
	return ret;
}

void
cainteoir_settings_set_boolean(CainteoirSettings *settings,
                               const gchar *group,
                               const gchar *key,
                               gboolean value)
{
	g_key_file_set_boolean(CAINTEOIR_SETTINGS_PRIVATE(settings)->settings, group, key, value);
}

gint
cainteoir_settings_get_integer(CainteoirSettings *settings,
                               const gchar *group,
                               const gchar *key,
                               gint default_value)
{
	GError *error = nullptr;
	gint ret = g_key_file_get_integer(CAINTEOIR_SETTINGS_PRIVATE(settings)->settings, group, key, &error);
	if (error)
	{
		g_error_free(error);
		return default_value;
	}
	return ret;
}

void
cainteoir_settings_set_integer(CainteoirSettings *settings,
                               const gchar *group,
                               const gchar *key,
                               gint value)
{
	g_key_file_set_integer(CAINTEOIR_SETTINGS_PRIVATE(settings)->settings, group, key, value);
}
