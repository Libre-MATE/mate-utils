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

#ifndef _GSEARCHTOOL_CALLBACKS_H_
#define _GSEARCHTOOL_CALLBACKS_H_

#include <glib.h>
#include <gtk/gtk.h>

#include "eggsmclient.h"

G_BEGIN_DECLS

void version_cb(const gchar *option_name, const gchar *value, gpointer data,
                GError **error);
void quit_session_cb(EggSMClient *client, gpointer data);
void quit_cb(GtkWidget *widget, GdkEvent *event, gpointer data);
void click_close_cb(GtkWidget *widget, gpointer data);
void click_find_cb(GtkWidget *widget, gpointer data);
void click_stop_cb(GtkWidget *widget, gpointer data);
void click_help_cb(GtkWidget *widget, gpointer data);
void click_expander_cb(GObject *object, GParamSpec *param_spec, gpointer data);
void size_allocate_cb(GtkWidget *widget, GtkAllocation *allocation,
                      gpointer data);
void add_constraint_cb(GtkWidget *widget, gpointer data);
void remove_constraint_cb(GtkWidget *widget, gpointer data);
void constraint_activate_cb(GtkWidget *widget, gpointer data);
void constraint_update_info_cb(GtkWidget *widget, gpointer data);
void name_contains_activate_cb(GtkWidget *widget, gpointer data);
void look_in_folder_changed_cb(GtkWidget *widget, gpointer data);
void open_file_cb(GtkMenuItem *action, gpointer data);
void open_file_event_cb(GtkWidget *widget, GdkEventButton *event,
                        gpointer data);
void open_folder_cb(GtkAction *action, gpointer data);
void copy_path_cb(GtkAction *action, gpointer data);
void file_changed_cb(GFileMonitor *handle, const gchar *monitor_uri,
                     const gchar *info_uri, GFileMonitorEvent event_type,
                     gpointer data);
void move_to_trash_cb(GtkAction *action, gpointer data);
void drag_begin_file_cb(GtkWidget *widget, GdkDragContext *context,
                        gpointer data);
void drag_file_cb(GtkWidget *widget, GdkDragContext *context,
                  GtkSelectionData *selection_data, guint info, guint time,
                  gpointer data);
void show_file_selector_cb(GtkAction *action, gpointer data);
void save_results_cb(GtkWidget *chooser, gint response, gpointer data);
void save_session_cb(EggSMClient *client, GKeyFile *state_file,
                     gpointer client_data);
gboolean key_press_cb(GtkWidget *widget, GdkEventKey *event, gpointer data);
gboolean file_button_release_event_cb(GtkWidget *widget, GdkEventButton *event,
                                      gpointer data);
gboolean file_event_after_cb(GtkWidget *widget, GdkEventButton *event,
                             gpointer data);
gboolean file_button_press_event_cb(GtkWidget *widget, GdkEventButton *event,
                                    gpointer data);
gboolean file_key_press_event_cb(GtkWidget *widget, GdkEventKey *event,
                                 gpointer data);
gboolean file_motion_notify_cb(GtkWidget *widget, GdkEventMotion *event,
                               gpointer user_data);
gboolean file_leave_notify_cb(GtkWidget *widget, GdkEventCrossing *event,
                              gpointer user_data);
gboolean not_running_timeout_cb(gpointer data);

void disable_quick_search_cb(GtkWidget *dialog, gint response, gpointer data);
void single_click_to_activate_key_changed_cb(GSettings *settings, gchar *key,
                                             gpointer user_data);
void columns_changed_cb(GtkTreeView *treeview, gpointer user_data);
gboolean window_state_event_cb(GtkWidget *widget, GdkEventWindowState *event,
                               gpointer data);

G_END_DECLS

#endif /* _GSEARCHTOOL_CALLBACKS_H_ */
