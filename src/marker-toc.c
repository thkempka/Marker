/*
 * marker-toc.c
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

#include <string.h>

#include "marker-toc.h"

typedef struct
{
  gint   level;
  gint   line;    /* 0-based line in the document */
  gchar *title;
} TocHeading;

struct _MarkerToc
{
  GtkScrolledWindow  parent_instance;

  GtkListBox        *list;
  GArray            *headings;   /* of TocHeading */
};

G_DEFINE_TYPE (MarkerToc, marker_toc, GTK_TYPE_SCROLLED_WINDOW)

enum
{
  HEADING_ACTIVATED,
  N_SIGNALS
};

static guint toc_signals[N_SIGNALS] = { 0 };

static void
free_heading (gpointer data, gpointer user_data)
{
  TocHeading *h = data;
  g_clear_pointer (&h->title, g_free);
}

static void
clear_rows (MarkerToc *toc)
{
  GList *children, *iter;

  children = gtk_container_get_children (GTK_CONTAINER (toc->list));
  for (iter = children; iter != NULL; iter = iter->next)
    gtk_widget_destroy (GTK_WIDGET (iter->data));
  g_list_free (children);

  g_array_set_size (toc->headings, 0);
}

static void
append_heading (MarkerToc   *toc,
                gint         level,
                gint         line,
                const gchar *title_start,
                const gchar *line_end)
{
  TocHeading h;
  GtkWidget *label;
  GtkWidget *row;
  g_autofree gchar *title = NULL;
  g_autofree gchar *stripped = NULL;
  gsize len;

  /* strip trailing whitespace and optional closing ATX sequence */
  len = line_end - title_start;
  while (len > 0 && (title_start[len - 1] == ' ' || title_start[len - 1] == '\t'))
    len--;
  if (len > 1 && title_start[len - 1] == '#')
    {
      while (len > 0 && title_start[len - 1] == '#')
        len--;
      while (len > 0 && (title_start[len - 1] == ' ' || title_start[len - 1] == '\t'))
        len--;
    }
  title = g_strndup (title_start, len);
  if (*title == '\0')
    return;

  h.level = level;
  h.line = line;
  h.title = g_strdup (title);
  g_array_append_val (toc->headings, h);

  label = gtk_label_new (title);
  gtk_label_set_xalign (GTK_LABEL (label), 0.0);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
  gtk_widget_set_margin_start (label, (level - 1) * 12);
  gtk_widget_set_tooltip_text (label, title);

  row = gtk_list_box_row_new ();
  gtk_container_add (GTK_CONTAINER (row), label);
  gtk_widget_show_all (row);
  gtk_list_box_insert (GTK_LIST_BOX (toc->list), row, -1);
}

void
marker_toc_set_content (MarkerToc   *toc,
                        const gchar *text)
{
  const gchar *p;
  const gchar *line_start;
  gint line = 0;
  gboolean in_code_fence = FALSE;

  g_return_if_fail (MARKER_IS_TOC (toc));

  clear_rows (toc);
  if (text == NULL)
    return;

  p = text;
  line_start = p;

  while (TRUE)
    {
      const gchar *line_end = strchr (line_start, '\n');
      gboolean last = (line_end == NULL);
      const gchar *s = line_start;
      gint hashes = 0;

      if (line_end == NULL)
        line_end = text + strlen (text);

      /* skip whitespace at line start */
      while (s < line_end && (*s == ' ' || *s == '\t'))
        s++;

      /* fenced code blocks do not contain headings */
      if (g_str_has_prefix (s, "```") || g_str_has_prefix (s, "~~~"))
        in_code_fence = !in_code_fence;
      else if (!in_code_fence && *s == '#')
        {
          while (s < line_end && *s == '#' && hashes < 7)
            {
              hashes++;
              s++;
            }
          if (hashes >= 1 && hashes <= 6 &&
              (s == line_end || *s == ' ' || *s == '\t'))
            {
              while (s < line_end && (*s == ' ' || *s == '\t'))
                s++;
              append_heading (toc, hashes, line, s, line_end);
            }
        }

      if (last)
        break;

      line_start = line_end + 1;
      line++;
    }
}

static void
row_activated_cb (GtkListBoxRow *row,
                  gpointer       user_data)
{
  MarkerToc *toc = MARKER_TOC (user_data);
  gint index = gtk_list_box_row_get_index (row);

  if (index >= 0 && (guint) index < toc->headings->len)
    {
      TocHeading *h = &g_array_index (toc->headings, TocHeading, index);
      g_signal_emit (toc, toc_signals[HEADING_ACTIVATED], 0, h->line, index);
    }
}

static void
marker_toc_class_init (MarkerTocClass *klass)
{
  /**
   * MarkerToc::heading-activated:
   * @toc: the #MarkerToc
   * @line: the 0-based line of the heading in the document
   * @index: the ordinal of the heading among all headings of the document
   *
   * Emitted when the user selects a heading of the table of contents.
   */
  toc_signals[HEADING_ACTIVATED] =
    g_signal_new ("heading-activated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL, NULL,
                  G_TYPE_NONE, 2,
                  G_TYPE_INT, G_TYPE_INT);
}

static void
marker_toc_init (MarkerToc *toc)
{
  toc->headings = g_array_new (FALSE, FALSE, sizeof (TocHeading));
  g_array_set_clear_func (toc->headings, free_heading);

  toc->list = GTK_LIST_BOX (gtk_list_box_new ());
  gtk_list_box_set_selection_mode (toc->list, GTK_SELECTION_BROWSE);
  g_signal_connect (toc->list, "row-activated",
                    G_CALLBACK (row_activated_cb), toc);

  gtk_container_add (GTK_CONTAINER (toc), GTK_WIDGET (toc->list));
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (toc),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);

  gtk_widget_show_all (GTK_WIDGET (toc->list));
}

MarkerToc *
marker_toc_new (void)
{
  return g_object_new (MARKER_TYPE_TOC, NULL);
}
