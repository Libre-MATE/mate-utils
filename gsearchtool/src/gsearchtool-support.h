/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Copyright (C) 2002 The Free Software Foundation
 * Copyright (C) 2012-2021 MATE Developers
 *
 * This file is part of MATE Utils.
 *
 * MATE Utils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * MATE Utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MATE Utils.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors:
 *   Dennis Cranston <dennis_cranston@yahoo.com>
 *   George Lebl
 */

#ifndef _GSEARCHTOOL_SUPPORT_H_
#define _GSEARCHTOOL_SUPPORT_H_

#include <glib.h>

#include "gsearchtool.h"

#define ICON_SIZE 24

G_BEGIN_DECLS

gboolean is_path_hidden(const gchar *path);

gboolean is_quick_search_excluded_path(const gchar *path);

gboolean is_second_scan_excluded_path(const gchar *path);

gboolean compare_regex(const gchar *regex, const gchar *string);
gboolean limit_string_to_x_lines(GString *string, gint x);
gchar *escape_single_quotes(const gchar *string);

gchar *escape_double_quotes(const gchar *string);

gchar *backslash_backslash_characters(const gchar *string);

gchar *backslash_special_characters(const gchar *string);

gchar *remove_mnemonic_character(const gchar *string);

gchar *get_readable_date(const CajaDateFormat date_format_enum,
                         GDateTime *file_time_raw);
gchar *get_file_type_description(const gchar *file, GFileInfo *file_info);
GdkPixbuf *get_file_pixbuf(GSearchWindow *gsearch, GFileInfo *file_info);
gboolean open_file_with_filemanager(GtkWidget *window, const gchar *file);
gboolean open_file_with_application(GtkWidget *window, const gchar *file,
                                    GAppInfo *app);
gboolean launch_file(const gchar *file);

gchar *gsearchtool_get_unique_filename(const gchar *path, const gchar *suffix);
GtkWidget *gsearchtool_button_new_with_icon(const gchar *string,
                                            const gchar *icon_name);
GSList *gsearchtool_get_columns_order(GtkTreeView *treeview);

void gsearchtool_set_columns_order(GtkTreeView *treeview);

void gsearchtool_get_stored_window_geometry(gint *width, gint *height);
gchar *gsearchtool_get_next_duplicate_name(const gchar *basname);

G_END_DECLS

#endif /* _GSEARCHTOOL_SUPPORT_H */
