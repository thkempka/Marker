/*
 * marker-toc.h
 *
 * Copyright (C) 2026 Fabio Colacio
 *
 * Marker is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * Marker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Marker; see the file LICENSE.md. If not,
 * see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __MARKER_TOC_H__
#define __MARKER_TOC_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MARKER_TYPE_TOC (marker_toc_get_type ())

G_DECLARE_FINAL_TYPE (MarkerToc, marker_toc, MARKER, TOC, GtkScrolledWindow)

MarkerToc      *marker_toc_new                   (void);

/*
 * marker_toc_set_content:
 * @toc: a #MarkerToc
 * @text: the markdown source of the current document
 *
 * Reparses @text and rebuilds the heading list.
 */
void            marker_toc_set_content           (MarkerToc   *toc,
                                                  const gchar *text);

G_END_DECLS

#endif /* __MARKER_TOC_H__ */
