/* Copyright (C) 2005-2006 Fabio Marzocca <thesaltydog@gmail.com>
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
 */

#ifndef __BAOBAB_UTILS_H__
#define __BAOBAB_UTILS_H__

#include "baobab.h"

void baobab_get_filesystem(BaobabFS *fs);
gchar *dir_select(gboolean, GtkWidget *);
void on_toggled(GtkToggleButton *, gpointer);
void stop_scan(void);
gboolean show_bars(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter,
                   gpointer data);
void message(const gchar *primary_msg, const gchar *secondary_msg,
             GtkMessageType type, GtkWidget *parent);
gint messageyesno(const gchar *primary_msg, const gchar *secondary_msg,
                  GtkMessageType type, gchar *ok_button, GtkWidget *parent);
gboolean baobab_check_dir(GFile *);
void popupmenu_list(GtkTreePath *path, GdkEventButton *event,
                    gboolean can_trash);
void open_file_with_application(GFile *file);
gboolean can_trash_file(GFile *file);
gboolean trash_file(GFile *file);
gboolean baobab_help_display(GtkWindow *parent, const gchar *file_name,
                             const gchar *link_id);
gboolean is_virtual_filesystem(GFile *file);

#endif /* __BAOBAB_UTILS_H__ */
