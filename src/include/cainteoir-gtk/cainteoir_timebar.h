/* Display a timebar.
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

#ifndef CAINTEOIR_GTK_CAINTEOIR_TIMEBAR_H
#define CAINTEOIR_GTK_CAINTEOIR_TIMEBAR_H

G_BEGIN_DECLS

#define CAINTEOIR_TYPE_TIMEBAR \
	(cainteoir_timebar_get_type())
#define CAINTEOIR_TIMEBAR(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), CAINTEOIR_TYPE_TIMEBAR, CainteoirTimeBar))
#define CAINTEOIR_TIMEBAR_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), CAINTEOIR_TYPE_TIMEBAR, CainteoirTimeBarClass))
#define CAINTEOIR_IS_TIMEBAR(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), CAINTEOIR_TYPE_TIMEBAR))
#define CAINTEOIR_IS_TIMEBAR_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), CAINTEOIR_TYPE_TIMEBAR))
#define CAINTEOIR_TIMEBAR_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), CAINTEOIR_TYPE_TIMEBAR, CainteoirTimeBarClass))

typedef struct _CainteoirTimeBar        CainteoirTimeBar;
typedef struct _CainteoirTimeBarPrivate CainteoirTimeBarPrivate;
typedef struct _CainteoirTimeBarClass   CainteoirTimeBarClass;

struct _CainteoirTimeBar
{
	GtkBox widget;

	/*< private >*/
	CainteoirTimeBarPrivate *priv;
};

struct _CainteoirTimeBarClass
{
	GtkBoxClass parent_class;

	/* Padding for future expansion */
	void (*_reserved1)(void);
	void (*_reserved2)(void);
	void (*_reserved3)(void);
	void (*_reserved4)(void);
};

GType                                  cainteoir_timebar_get_type(void) G_GNUC_CONST;

GtkWidget *                            cainteoir_timebar_new(void);

void                                   cainteoir_timebar_set_time(CainteoirTimeBar *timebar,
                                                                  gdouble elapsed_time,
                                                                  gdouble total_time);

G_END_DECLS

#endif
