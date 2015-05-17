/* A GTK+ wrapper around cainteoir RDF metadata results.
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

#ifndef CAINTEOIR_GTK_CAINTEOIR_METADATA_H
#define CAINTEOIR_GTK_CAINTEOIR_METADATA_H

G_BEGIN_DECLS

#define CAINTEOIR_TYPE_METADATA \
	(cainteoir_metadata_get_type())
#define CAINTEOIR_METADATA(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), CAINTEOIR_TYPE_METADATA, CainteoirMetadata))
#define CAINTEOIR_METADATA_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), CAINTEOIR_TYPE_METADATA, CainteoirMetadataClass))
#define CAINTEOIR_IS_METADATA(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), CAINTEOIR_TYPE_METADATA))
#define CAINTEOIR_IS_METADATA_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), CAINTEOIR_TYPE_METADATA))
#define CAINTEOIR_METADATA_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), CAINTEOIR_TYPE_METADATA, CainteoirMetadataClass))

typedef struct _CainteoirMetadata        CainteoirMetadata;
typedef struct _CainteoirMetadataPrivate CainteoirMetadataPrivate;
typedef struct _CainteoirMetadataClass   CainteoirMetadataClass;

struct _CainteoirMetadata
{
	GObject parent_instance;

	/*< private >*/
	CainteoirMetadataPrivate *priv;
};

struct _CainteoirMetadataClass
{
	GObjectClass parent_class;

	/* Padding for future expansion */
	void (*_reserved1)(void);
	void (*_reserved2)(void);
	void (*_reserved3)(void);
	void (*_reserved4)(void);
};

GType                                  cainteoir_metadata_get_type(void) G_GNUC_CONST;

G_END_DECLS

#endif
