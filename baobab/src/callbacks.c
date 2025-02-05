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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gio/gio.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <string.h>

#include "baobab-chart.h"
#include "baobab-prefs.h"
#include "baobab-remote-connect-dialog.h"
#include "baobab-treeview.h"
#include "baobab-utils.h"
#include "baobab.h"
#include "callbacks.h"

void on_quit_activate(GtkMenuItem *menuitem, gpointer user_data) {
  baobab_quit();
}

void on_menuscanhome_activate(GtkMenuItem *menuitem, gpointer user_data) {
  baobab_scan_home();
}

void on_menuallfs_activate(GtkMenuItem *menuitem, gpointer user_data) {
  baobab_scan_root();
}

void on_menuscandir_activate(GtkMenuItem *menuitem, gpointer user_data) {
  dir_select(FALSE, baobab.window);
}

void on_about_activate(GtkMenuItem *menuitem, gpointer user_data) {
  const gchar *const authors[] = {
      "Fabio Marzocca <thesaltydog@gmail.com>",
      "Paolo Borelli <pborelli@katamail.com>",
      "Benoît Dejean <benoit@placenet.org>",
      "Igalia (rings-chart and treemap widget) <www.igalia.com>", NULL};

  const gchar *documenters[] = {
      "Fabio Marzocca <thesaltydog@gmail.com>",
      N_("MATE Documentation Team"),
      NULL,
  };

  const gchar *license[] = {
      N_("This program is free software; you can redistribute it and/or "
         "modify it under the terms of the GNU General Public License as "
         "published by the Free Software Foundation; either version 2 of "
         "the License, or (at your option) any later version."),

      N_("This program is distributed in the hope that it will be useful, "
         "but WITHOUT ANY WARRANTY; without even the implied warranty of "
         "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU "
         "General Public License for more details."),

      N_("You should have received a copy of the GNU General Public License "
         "along with this program.  If not, see "
         "<https://www.gnu.org/licenses/>.")};

  gchar *license_trans =
      g_strjoin("\n\n", _(license[0]), _(license[1]), _(license[2]), NULL);

#ifdef ENABLE_NLS
  const char **p;
  for (p = documenters; *p; ++p) *p = _(*p);
#endif

  gtk_show_about_dialog(
      GTK_WINDOW(baobab.window), "program-name", _("Disk Usage Analyzer"),
      "version", VERSION, "title", _("About Disk Usage Analyzer"), "comments",
      _("Analyze your disk usage through a graphical tool."), "copyright",
      _("Copyright \xc2\xa9 2005-2010 Fabio Marzocca\n"
        "Copyright \xc2\xa9 2011-2021 MATE developers\n"
        "Copyright \xc2\xa9 2022 Libre MATE"),
      "logo-icon-name", "mate-disk-usage-analyzer", "license", license_trans,
      "authors", authors, "documenters", documenters, "translator-credits",
      _("translator-credits"), "wrap-license", TRUE, NULL);

  g_free(license_trans);
}

void on_menu_expand_activate(GtkMenuItem *menuitem, gpointer user_data) {
  gtk_tree_view_expand_all(GTK_TREE_VIEW(baobab.tree_view));
}

void on_menu_collapse_activate(GtkMenuItem *menuitem, gpointer user_data) {
  gtk_tree_view_collapse_all(GTK_TREE_VIEW(baobab.tree_view));
}

void on_menu_stop_activate(GtkMenuItem *menuitem, gpointer user_data) {
  baobab_stop_scan();
}

void on_menu_rescan_activate(GtkMenuItem *menuitem, gpointer user_data) {
  baobab_rescan_current_dir();
}

void on_tbscandir_clicked(GtkToolButton *toolbutton, gpointer user_data) {
  dir_select(FALSE, baobab.window);
}

void on_tbscanhome_clicked(GtkToolButton *toolbutton, gpointer user_data) {
  baobab_scan_home();
}

void on_tbscanall_clicked(GtkToolButton *toolbutton, gpointer user_data) {
  baobab_scan_root();
}

void on_tb_scan_remote_clicked(GtkToolButton *toolbutton, gpointer user_data) {
  GtkWidget *dlg;

  dlg = baobab_remote_connect_dialog_new(GTK_WINDOW(baobab.window), NULL);
  gtk_dialog_run(GTK_DIALOG(dlg));

  gtk_widget_destroy(dlg);
}

void on_menu_scan_rem_activate(GtkMenuItem *menuitem, gpointer user_data) {
  on_tb_scan_remote_clicked(NULL, NULL);
}

void on_tbstop_clicked(GtkToolButton *toolbutton, gpointer user_data) {
  baobab_stop_scan();
}

void on_tbrescan_clicked(GtkToolButton *toolbutton, gpointer user_data) {
  baobab_rescan_current_dir();
}

gboolean on_delete_activate(GtkWidget *widget, GdkEvent *event,
                            gpointer user_data) {
  baobab_quit();
  return TRUE;
}

void open_file_cb(GtkMenuItem *pmenu, gpointer dummy) {
  GFile *file;

  g_assert(!dummy);
  g_assert(baobab.selected_path);

  file = g_file_parse_name(baobab.selected_path);

  if (!g_file_query_exists(file, NULL)) {
    message(_("The document does not exist."), "", GTK_MESSAGE_INFO,
            baobab.window);
    g_object_unref(file);
    return;
  }

  open_file_with_application(file);
  g_object_unref(file);
}

void trash_dir_cb(GtkMenuItem *pmenu, gpointer dummy) {
  GFile *file;

  g_assert(!dummy);
  g_assert(baobab.selected_path);

  file = g_file_parse_name(baobab.selected_path);

  if (trash_file(file)) {
    GtkTreeIter iter;
    guint64 filesize;
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection((GtkTreeView *)baobab.tree_view);
    gtk_tree_selection_get_selected(selection, NULL, &iter);
    gtk_tree_model_get((GtkTreeModel *)baobab.model, &iter, 5, &filesize, -1);
    gtk_tree_store_remove(GTK_TREE_STORE(baobab.model), &iter);
  }

  g_object_unref(file);
}

void on_pref_menu(GtkAction *a, gpointer user_data) { baobab_prefs_dialog(); }

void on_ck_allocated_activate(GtkToggleAction *action, gpointer user_data) {
  if (!baobab.is_local) return;

  baobab.show_allocated = gtk_toggle_action_get_active(action);

  baobab_treeview_show_allocated_size(baobab.tree_view, baobab.show_allocated);

  baobab_set_busy(TRUE);
  baobab_set_statusbar(_("Calculating percentage bars..."));
  gtk_tree_model_foreach(GTK_TREE_MODEL(baobab.model), show_bars, NULL);
  baobab_set_busy(FALSE);
  baobab_set_statusbar(_("Ready"));
}

void on_helpcontents_activate(GtkAction *a, gpointer user_data) {
  baobab_help_display(GTK_WINDOW(baobab.window), "mate-disk-usage-analyzer",
                      NULL);
}

void scan_folder_cb(GtkMenuItem *pmenu, gpointer dummy) {
  GFile *file;

  g_assert(!dummy);
  g_assert(baobab.selected_path);

  file = g_file_parse_name(baobab.selected_path);

  if (!g_file_query_exists(file, NULL)) {
    message(_("The folder does not exist."), "", GTK_MESSAGE_INFO,
            baobab.window);
  }

  baobab_scan_location(file);
  g_object_unref(file);
}

void on_tv_selection_changed(GtkTreeSelection *selection, gpointer user_data) {
  GtkTreeIter iter;

  if (gtk_tree_selection_get_selected(selection, NULL, &iter)) {
    GtkTreePath *path;

    path = gtk_tree_model_get_path(GTK_TREE_MODEL(baobab.model), &iter);

    baobab_chart_set_root(baobab.rings_chart, path);
    baobab_chart_set_root(baobab.treemap_chart, path);

    gtk_tree_path_free(path);
  }
}

void on_move_upwards_cb(GtkCheckMenuItem *checkmenuitem, gpointer user_data) {
  baobab_chart_move_up_root(baobab.current_chart);
}

void on_zoom_in_cb(GtkCheckMenuItem *checkmenuitem, gpointer user_data) {
  baobab_chart_zoom_in(baobab.current_chart);
}

void on_zoom_out_cb(GtkCheckMenuItem *checkmenuitem, gpointer user_data) {
  baobab_chart_zoom_out(baobab.current_chart);
}

void on_chart_snapshot_cb(GtkCheckMenuItem *checkmenuitem, gpointer user_data) {
  baobab_chart_save_snapshot(baobab.current_chart);
}
