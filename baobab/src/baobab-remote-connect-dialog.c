/* Copyright (C) 2005 Fabio Marzocca
 * Copyright (C) 2012-2021 MATE Developers
 *
 * Modified module from caja-connect-server-dialog.c.
 * Released under same licence.
 * Copyright (C) 2003 Red Hat, Inc.
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

#include "baobab-remote-connect-dialog.h"
#include "baobab.h"

/* Translators: all the strings in this file are meant to map the
   similar strings inside caja-connect-server and should be
   translated the same way
*/

struct _BaobabRemoteConnectDialogDetails {
  GtkWidget *grid;
  GtkWidget *type_combo;
  GtkWidget *uri_entry;
  GtkWidget *server_entry;
  GtkWidget *share_entry;
  GtkWidget *port_entry;
  GtkWidget *folder_entry;
  GtkWidget *domain_entry;
  GtkWidget *user_entry;
};

G_DEFINE_TYPE(BaobabRemoteConnectDialog, baobab_remote_connect_dialog,
              GTK_TYPE_DIALOG)

#define RESPONSE_CONNECT GTK_RESPONSE_OK

static void display_error_dialog(GError *error, GFile *location,
                                 GtkWindow *parent) {
  GtkWidget *dlg;
  char *parse_name;

  parse_name = g_file_get_parse_name(location);
  dlg = gtk_message_dialog_new(parent, GTK_DIALOG_DESTROY_WITH_PARENT,
                               GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                               _("Cannot scan location \"%s\""), parse_name);
  g_free(parse_name);

  gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dlg), "%s",
                                           error->message);

  gtk_dialog_run(GTK_DIALOG(dlg));
  gtk_widget_destroy(dlg);
}

static void mount_enclosing_ready_cb(GFile *location, GAsyncResult *res,
                                     GtkWindow *app) {
  gboolean success;
  GError *error = NULL;

  success = g_file_mount_enclosing_volume_finish(location, res, &error);

  if (success ||
      g_error_matches(error, G_IO_ERROR, G_IO_ERROR_ALREADY_MOUNTED)) {
    baobab_scan_location(location);
  } else {
    display_error_dialog(error, location, app);
  }

  if (error) g_error_free(error);

  if (location) g_object_unref(location);
}

static void connect_server_dialog_present_uri(GtkWindow *app, GFile *location,
                                              GtkWidget *widget) {
  GMountOperation *op;

  op = gtk_mount_operation_new(GTK_WINDOW(widget));

  g_file_mount_enclosing_volume(location, 0, op, NULL,
                                (GAsyncReadyCallback)mount_enclosing_ready_cb,
                                app);
  g_object_unref(op);
}

struct MethodInfo {
  const char *scheme;
  guint flags;
};

/* A collection of flags for MethodInfo.flags */
enum {
  DEFAULT_METHOD = 0x00000001,

  /* Widgets to display in setup_for_type */
  SHOW_SHARE = 0x00000010,
  SHOW_PORT = 0x00000020,
  SHOW_USER = 0x00000040,
  SHOW_DOMAIN = 0x00000080,

  IS_ANONYMOUS = 0x00001000
};

/* Remember to fill in descriptions below */
static struct MethodInfo methods[] = {
    /* FIXME: we need to alias ssh to sftp */
    {"sftp", SHOW_PORT | SHOW_USER},
    {"ftp", SHOW_PORT | SHOW_USER},
    {"ftp", DEFAULT_METHOD | IS_ANONYMOUS | SHOW_PORT},
    {"smb", SHOW_SHARE | SHOW_USER | SHOW_DOMAIN},
    {"dav", SHOW_PORT | SHOW_USER},
    /* FIXME: hrm, shouldn't it work? */
    {"davs", SHOW_PORT | SHOW_USER},
    {NULL, 0}, /* Custom URI method */
};

/* To get around non constant gettext strings */
static const char *get_method_description(struct MethodInfo *meth) {
  if (!meth->scheme) {
    return _("Custom Location");
  } else if (strcmp(meth->scheme, "sftp") == 0) {
    return _("SSH");
  } else if (strcmp(meth->scheme, "ftp") == 0) {
    if (meth->flags & IS_ANONYMOUS) {
      return _("Public FTP");
    } else {
      return _("FTP (with login)");
    }
  } else if (strcmp(meth->scheme, "smb") == 0) {
    return _("Windows share");
  } else if (strcmp(meth->scheme, "dav") == 0) {
    return _("WebDAV (HTTP)");
  } else if (strcmp(meth->scheme, "davs") == 0) {
    return _("Secure WebDAV (HTTPS)");

    /* No descriptive text */
  } else {
    return meth->scheme;
  }
}

static void baobab_remote_connect_dialog_finalize(GObject *object) {
  BaobabRemoteConnectDialog *dialog;

  dialog = BAOBAB_REMOTE_CONNECT_DIALOG(object);

  g_object_unref(dialog->details->uri_entry);
  g_object_unref(dialog->details->server_entry);
  g_object_unref(dialog->details->share_entry);
  g_object_unref(dialog->details->port_entry);
  g_object_unref(dialog->details->folder_entry);
  g_object_unref(dialog->details->domain_entry);
  g_object_unref(dialog->details->user_entry);

  g_free(dialog->details);

  G_OBJECT_CLASS(baobab_remote_connect_dialog_parent_class)->finalize(object);
}

static void connect_to_server(BaobabRemoteConnectDialog *dialog,
                              GtkWindow *app) {
  struct MethodInfo *meth;
  char *uri;
  GFile *location;
  int index;
  GtkTreeIter iter;

  /* Get our method info */
  gtk_combo_box_get_active_iter(GTK_COMBO_BOX(dialog->details->type_combo),
                                &iter);
  gtk_tree_model_get(
      gtk_combo_box_get_model(GTK_COMBO_BOX(dialog->details->type_combo)),
      &iter, 0, &index, -1);
  g_assert(index < G_N_ELEMENTS(methods) && index >= 0);
  meth = &(methods[index]);

  if (meth->scheme == NULL) {
    uri =
        gtk_editable_get_chars(GTK_EDITABLE(dialog->details->uri_entry), 0, -1);
    /* FIXME: we should validate it in some way? */
  } else {
    char *user, *port, *initial_path, *server, *folder, *domain;
    char *t, *join;
    gboolean free_initial_path, free_user, free_domain, free_port;

    server = gtk_editable_get_chars(GTK_EDITABLE(dialog->details->server_entry),
                                    0, -1);
    if (strlen(server) == 0) {
      GtkWidget *dlg;

      dlg = gtk_message_dialog_new(
          GTK_WINDOW(dialog), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
          GTK_BUTTONS_OK,
          _("Cannot Connect to Server. You must enter a name for the server."));

      gtk_message_dialog_format_secondary_text(
          GTK_MESSAGE_DIALOG(dlg), _("Please enter a name and try again."));

      gtk_dialog_run(GTK_DIALOG(dlg));
      gtk_widget_destroy(dlg);

      g_free(server);
      return;
    }

    user = "";
    port = "";
    initial_path = "";
    domain = "";
    free_initial_path = FALSE;
    free_user = FALSE;
    free_domain = FALSE;
    free_port = FALSE;

    /* FTP special case */
    if (meth->flags & IS_ANONYMOUS) {
      user = "anonymous";

      /* SMB special case */
    } else if (strcmp(meth->scheme, "smb") == 0) {
      t = gtk_editable_get_chars(GTK_EDITABLE(dialog->details->share_entry), 0,
                                 -1);
      initial_path = g_strconcat("/", t, NULL);
      free_initial_path = TRUE;
      g_free(t);
    }

    if (gtk_widget_get_parent(dialog->details->port_entry) != NULL) {
      free_port = TRUE;
      port = gtk_editable_get_chars(GTK_EDITABLE(dialog->details->port_entry),
                                    0, -1);
    }
    folder = gtk_editable_get_chars(GTK_EDITABLE(dialog->details->folder_entry),
                                    0, -1);
    if (gtk_widget_get_parent(dialog->details->user_entry) != NULL) {
      free_user = TRUE;

      t = gtk_editable_get_chars(GTK_EDITABLE(dialog->details->user_entry), 0,
                                 -1);

      user = g_uri_escape_string(t, G_URI_RESERVED_CHARS_ALLOWED_IN_USERINFO,
                                 FALSE);

      g_free(t);
    }
    if (gtk_widget_get_parent(dialog->details->domain_entry) != NULL) {
      free_domain = TRUE;

      domain = gtk_editable_get_chars(
          GTK_EDITABLE(dialog->details->domain_entry), 0, -1);

      if (strlen(domain) != 0) {
        t = user;

        user = g_strconcat(domain, ";", t, NULL);

        if (free_user) {
          g_free(t);
        }

        free_user = TRUE;
      }
    }

    if (folder[0] != 0 && folder[0] != '/') {
      join = "/";
    } else {
      join = "";
    }

    t = folder;
    folder = g_strconcat(initial_path, join, t, NULL);
    g_free(t);

    t = folder;
    folder =
        g_uri_escape_string(t, G_URI_RESERVED_CHARS_ALLOWED_IN_PATH, FALSE);
    g_free(t);

    uri = g_strdup_printf("%s://%s%s%s%s%s%s", meth->scheme, user,
                          (user[0] != 0) ? "@" : "", server,
                          (port[0] != 0) ? ":" : "", port, folder);

    if (free_initial_path) {
      g_free(initial_path);
    }
    g_free(server);
    if (free_port) {
      g_free(port);
    }
    g_free(folder);
    if (free_user) {
      g_free(user);
    }
    if (free_domain) {
      g_free(domain);
    }
  }

  gtk_widget_hide(GTK_WIDGET(dialog));

  location = g_file_new_for_uri(uri);
  g_free(uri);

  connect_server_dialog_present_uri(app, location, GTK_WIDGET(dialog));
}

static void response_callback(BaobabRemoteConnectDialog *dialog,
                              int response_id, GtkWindow *app) {
  switch (response_id) {
    case RESPONSE_CONNECT:
      connect_to_server(dialog, app);
      break;
    case GTK_RESPONSE_NONE:
    case GTK_RESPONSE_DELETE_EVENT:
    case GTK_RESPONSE_CANCEL:
      break;
    default:
      g_assert_not_reached();
  }
}

static void baobab_remote_connect_dialog_class_init(
    BaobabRemoteConnectDialogClass *class) {
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS(class);
  gobject_class->finalize = baobab_remote_connect_dialog_finalize;
}
static void destroy_label(GtkWidget *widget, gpointer user_data) {
  gtk_widget_destroy(widget);
}

static void setup_for_type(BaobabRemoteConnectDialog *dialog) {
  struct MethodInfo *meth;
  int index, i;
  GtkWidget *label, *grid;
  GtkTreeIter iter;

  /* Get our method info */
  gtk_combo_box_get_active_iter(GTK_COMBO_BOX(dialog->details->type_combo),
                                &iter);
  gtk_tree_model_get(
      gtk_combo_box_get_model(GTK_COMBO_BOX(dialog->details->type_combo)),
      &iter, 0, &index, -1);
  g_assert(index < G_N_ELEMENTS(methods) && index >= 0);
  meth = &(methods[index]);

  if (gtk_widget_get_parent(dialog->details->uri_entry) != NULL) {
    gtk_container_remove(GTK_CONTAINER(dialog->details->grid),
                         dialog->details->uri_entry);
  }
  if (gtk_widget_get_parent(dialog->details->server_entry) != NULL) {
    gtk_container_remove(GTK_CONTAINER(dialog->details->grid),
                         dialog->details->server_entry);
  }
  if (gtk_widget_get_parent(dialog->details->share_entry) != NULL) {
    gtk_container_remove(GTK_CONTAINER(dialog->details->grid),
                         dialog->details->share_entry);
  }
  if (gtk_widget_get_parent(dialog->details->port_entry) != NULL) {
    gtk_container_remove(GTK_CONTAINER(dialog->details->grid),
                         dialog->details->port_entry);
  }
  if (gtk_widget_get_parent(dialog->details->folder_entry) != NULL) {
    gtk_container_remove(GTK_CONTAINER(dialog->details->grid),
                         dialog->details->folder_entry);
  }
  if (gtk_widget_get_parent(dialog->details->user_entry) != NULL) {
    gtk_container_remove(GTK_CONTAINER(dialog->details->grid),
                         dialog->details->user_entry);
  }
  if (gtk_widget_get_parent(dialog->details->domain_entry) != NULL) {
    gtk_container_remove(GTK_CONTAINER(dialog->details->grid),
                         dialog->details->domain_entry);
  }
  /* Destroy all labels */
  gtk_container_foreach(GTK_CONTAINER(dialog->details->grid), destroy_label,
                        NULL);

  i = 1;
  grid = dialog->details->grid;

  if (meth->scheme == NULL) {
    label = gtk_label_new_with_mnemonic(_("_Location (URI):"));
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_label_set_yalign(GTK_LABEL(label), 0.5);
    gtk_widget_show(label);
    gtk_grid_attach(GTK_GRID(grid), label, 0, i, 1, 1);

    gtk_label_set_mnemonic_widget(GTK_LABEL(label), dialog->details->uri_entry);
    gtk_widget_set_hexpand(dialog->details->uri_entry, TRUE);
    gtk_widget_show(dialog->details->uri_entry);
    gtk_grid_attach(GTK_GRID(grid), dialog->details->uri_entry, 1, i, 1, 1);

    i++;

    return;
  }

  label = gtk_label_new_with_mnemonic(_("_Server:"));
  gtk_label_set_xalign(GTK_LABEL(label), 0.0);
  gtk_label_set_yalign(GTK_LABEL(label), 0.5);
  gtk_widget_show(label);
  gtk_grid_attach(GTK_GRID(grid), label, 0, i, 1, 1);

  gtk_label_set_mnemonic_widget(GTK_LABEL(label),
                                dialog->details->server_entry);
  gtk_widget_set_hexpand(dialog->details->server_entry, TRUE);
  gtk_widget_show(dialog->details->server_entry);
  gtk_grid_attach(GTK_GRID(grid), dialog->details->server_entry, 1, i, 1, 1);

  i++;

  label = gtk_label_new(_("Optional information:"));
  gtk_label_set_xalign(GTK_LABEL(label), 0.0);
  gtk_label_set_yalign(GTK_LABEL(label), 0.5);
  gtk_widget_show(label);
  gtk_grid_attach(GTK_GRID(grid), label, 0, i, 2, 1);

  i++;

  if (meth->flags & SHOW_SHARE) {
    label = gtk_label_new_with_mnemonic(_("_Share:"));
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_label_set_yalign(GTK_LABEL(label), 0.5);
    gtk_widget_show(label);
    gtk_grid_attach(GTK_GRID(grid), label, 0, i, 1, 1);

    gtk_label_set_mnemonic_widget(GTK_LABEL(label),
                                  dialog->details->share_entry);
    gtk_widget_set_hexpand(dialog->details->share_entry, TRUE);
    gtk_widget_show(dialog->details->share_entry);
    gtk_grid_attach(GTK_GRID(grid), dialog->details->share_entry, 1, i, 1, 1);

    i++;
  }

  if (meth->flags & SHOW_PORT) {
    label = gtk_label_new_with_mnemonic(_("_Port:"));
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_label_set_yalign(GTK_LABEL(label), 0.5);
    gtk_widget_show(label);
    gtk_grid_attach(GTK_GRID(grid), label, 0, i, 1, 1);

    gtk_label_set_mnemonic_widget(GTK_LABEL(label),
                                  dialog->details->port_entry);
    gtk_widget_set_hexpand(dialog->details->port_entry, TRUE);
    gtk_widget_show(dialog->details->port_entry);
    gtk_grid_attach(GTK_GRID(grid), dialog->details->port_entry, 1, i, 1, 1);

    i++;
  }

  label = gtk_label_new_with_mnemonic(_("_Folder:"));
  gtk_label_set_xalign(GTK_LABEL(label), 0.0);
  gtk_label_set_yalign(GTK_LABEL(label), 0.5);
  gtk_widget_show(label);
  gtk_grid_attach(GTK_GRID(grid), label, 0, i, 1, 1);

  gtk_label_set_mnemonic_widget(GTK_LABEL(label),
                                dialog->details->folder_entry);
  gtk_widget_set_hexpand(dialog->details->folder_entry, TRUE);
  gtk_widget_show(dialog->details->folder_entry);
  gtk_grid_attach(GTK_GRID(grid), dialog->details->folder_entry, 1, i, 1, 1);

  i++;

  if (meth->flags & SHOW_USER) {
    label = gtk_label_new_with_mnemonic(_("_User Name:"));
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_label_set_yalign(GTK_LABEL(label), 0.5);
    gtk_widget_show(label);
    gtk_grid_attach(GTK_GRID(grid), label, 0, i, 1, 1);

    gtk_label_set_mnemonic_widget(GTK_LABEL(label),
                                  dialog->details->user_entry);
    gtk_widget_set_hexpand(dialog->details->user_entry, TRUE);
    gtk_widget_show(dialog->details->user_entry);
    gtk_grid_attach(GTK_GRID(grid), dialog->details->user_entry, 1, i, 1, 1);

    i++;
  }

  if (meth->flags & SHOW_DOMAIN) {
    label = gtk_label_new_with_mnemonic(_("_Domain Name:"));
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_label_set_yalign(GTK_LABEL(label), 0.5);
    gtk_widget_show(label);
    gtk_grid_attach(GTK_GRID(grid), label, 0, i, 1, 1);

    gtk_label_set_mnemonic_widget(GTK_LABEL(label),
                                  dialog->details->domain_entry);
    gtk_widget_set_hexpand(dialog->details->user_entry, TRUE);
    gtk_widget_show(dialog->details->domain_entry);
    gtk_grid_attach(GTK_GRID(grid), dialog->details->domain_entry, 1, i, 1, 1);

    i++;
  }
}

static void combo_changed_callback(GtkComboBox *combo_box,
                                   BaobabRemoteConnectDialog *dialog) {
  setup_for_type(dialog);
}

static void port_insert_text(GtkEditable *editable, const gchar *new_text,
                             gint new_text_length, gint *position) {
  int pos;

  /* Only allow digits to be inserted as port number */
  for (pos = 0; pos < new_text_length; pos++) {
    if (!g_ascii_isdigit(new_text[pos])) {
      GtkWidget *toplevel = gtk_widget_get_toplevel(GTK_WIDGET(editable));
      if (toplevel != NULL) {
        gdk_window_beep(gtk_widget_get_window(toplevel));
      }
      g_signal_stop_emission_by_name(editable, "insert_text");
      return;
    }
  }
}

static void baobab_remote_connect_dialog_init(
    BaobabRemoteConnectDialog *dialog) {
  GtkWidget *label;
  GtkWidget *grid;
  GtkWidget *combo;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkListStore *store;
  GtkCellRenderer *renderer;
  int i;

  dialog->details = g_new0(BaobabRemoteConnectDialogDetails, 1);

  gtk_window_set_title(GTK_WINDOW(dialog), _("Connect to Server"));
  gtk_container_set_border_width(GTK_CONTAINER(dialog), 5);
  gtk_box_set_spacing(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                      2);
  gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                     vbox, FALSE, TRUE, 0);
  gtk_widget_show(vbox);

  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
  gtk_widget_show(hbox);

  label = gtk_label_new_with_mnemonic(_("Service _type:"));
  gtk_label_set_xalign(GTK_LABEL(label), 0.0);
  gtk_label_set_yalign(GTK_LABEL(label), 0.5);
  gtk_widget_show(label);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  dialog->details->type_combo = combo = gtk_combo_box_new();

  /* each row contains: method index, textual description */
  store = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);
  gtk_combo_box_set_model(GTK_COMBO_BOX(combo), GTK_TREE_MODEL(store));
  g_object_unref(G_OBJECT(store));

  renderer = gtk_cell_renderer_text_new();
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, TRUE);
  gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(combo), renderer, "text", 1);

  for (i = 0; i < G_N_ELEMENTS(methods); i++) {
    GtkTreeIter iter;
    const gchar *const *supported;
    int j;

    /* skip methods that don't have corresponding MateVFSMethods */
    supported = g_vfs_get_supported_uri_schemes(g_vfs_get_default());

    if (methods[i].scheme != NULL) {
      gboolean found;

      found = FALSE;
      for (j = 0; supported[j] != NULL; j++) {
        if (strcmp(methods[i].scheme, supported[j]) == 0) {
          found = TRUE;
          break;
        }
      }

      if (!found) {
        continue;
      }
    }

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, i, 1,
                       get_method_description(&(methods[i])), -1);

    if (methods[i].flags & DEFAULT_METHOD) {
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo), &iter);
    }
  }

  if (gtk_combo_box_get_active(GTK_COMBO_BOX(combo)) < 0) {
    /* default method not available, use any other */
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
  }

  gtk_widget_show(combo);
  gtk_label_set_mnemonic_widget(GTK_LABEL(label), combo);
  gtk_box_pack_start(GTK_BOX(hbox), combo, TRUE, TRUE, 0);
  g_signal_connect(combo, "changed", G_CALLBACK(combo_changed_callback),
                   dialog);

  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
  gtk_widget_show(hbox);

  label = gtk_label_new_with_mnemonic("    ");
  gtk_widget_show(label);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  dialog->details->grid = grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
  gtk_widget_show(grid);
  gtk_box_pack_start(GTK_BOX(hbox), grid, TRUE, TRUE, 0);

  dialog->details->uri_entry = gtk_entry_new();
  dialog->details->server_entry = gtk_entry_new();
  dialog->details->share_entry = gtk_entry_new();
  dialog->details->port_entry = gtk_entry_new();
  g_signal_connect(dialog->details->port_entry, "insert_text",
                   G_CALLBACK(port_insert_text), NULL);
  dialog->details->folder_entry = gtk_entry_new();
  dialog->details->domain_entry = gtk_entry_new();
  dialog->details->user_entry = gtk_entry_new();

  gtk_entry_set_activates_default(GTK_ENTRY(dialog->details->uri_entry), TRUE);
  gtk_entry_set_activates_default(GTK_ENTRY(dialog->details->server_entry),
                                  TRUE);
  gtk_entry_set_activates_default(GTK_ENTRY(dialog->details->share_entry),
                                  TRUE);
  gtk_entry_set_activates_default(GTK_ENTRY(dialog->details->port_entry), TRUE);
  gtk_entry_set_activates_default(GTK_ENTRY(dialog->details->folder_entry),
                                  TRUE);
  gtk_entry_set_activates_default(GTK_ENTRY(dialog->details->domain_entry),
                                  TRUE);
  gtk_entry_set_activates_default(GTK_ENTRY(dialog->details->user_entry), TRUE);

  /* We need an extra ref so we can remove them from the table */
  g_object_ref(dialog->details->uri_entry);
  g_object_ref(dialog->details->server_entry);
  g_object_ref(dialog->details->share_entry);
  g_object_ref(dialog->details->port_entry);
  g_object_ref(dialog->details->folder_entry);
  g_object_ref(dialog->details->domain_entry);
  g_object_ref(dialog->details->user_entry);

  setup_for_type(dialog);

  gtk_dialog_add_button(GTK_DIALOG(dialog), "gtk-cancel", GTK_RESPONSE_CANCEL);
  gtk_dialog_add_button(GTK_DIALOG(dialog), _("_Scan"), RESPONSE_CONNECT);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), RESPONSE_CONNECT);
}

GtkWidget *baobab_remote_connect_dialog_new(GtkWindow *window,
                                            GFile *location) {
  GtkWidget *dialog;

  dialog = gtk_widget_new(BAOBAB_TYPE_REMOTE_CONNECT_DIALOG, NULL);

  gtk_window_set_transient_for(GTK_WINDOW(dialog), window);

  g_signal_connect(dialog, "response", G_CALLBACK(response_callback), window);

  if (window) {
    gtk_window_set_screen(GTK_WINDOW(dialog),
                          gtk_window_get_screen(GTK_WINDOW(window)));
  }

  return dialog;
}
