/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2; -*- */
/* Copyright (C) 2005 Vincent Noel <vnoel@cox.net>
 * Copyright (C) 2008 Cosimo Cecchi <cosimoc@gnome.org>
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

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <stdlib.h>

#include "logview-app.h"

/* log files specified on the command line */
static char **log_files = NULL;

static void app_quit_cb(LogviewApp *app, gpointer user_data) {
  gtk_main_quit();
}

static void logview_show_version_and_quit(void) {
  g_print(_("%s - Version %s\n"
            "Copyright (C) 2004-2008 Vincent Noel, Cosimo Cecchi and others.\n"
            "Copyright (C) 2011-2021 MATE developers.\n"
            "Copyright (C) 2022 Libre MATE.\n"),
          g_get_application_name(), VERSION);

  exit(0);
}

static GOptionContext *create_option_context(void) {
  GOptionContext *context;

  const GOptionEntry entries[] = {
      {"version", 'V', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
       logview_show_version_and_quit, N_("Show the application's version"),
       NULL},
      {G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_FILENAME_ARRAY, &log_files,
       NULL, N_("[LOGFILE...]")},
      {NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL},
  };

  context = g_option_context_new(_(" - Browse and monitor logs"));
#ifdef ENABLE_NLS
  g_option_context_set_translation_domain(context, GETTEXT_PACKAGE);
#endif /* ENABLE_NLS */
  g_option_context_add_main_entries(context, entries, GETTEXT_PACKAGE);
  g_option_context_set_ignore_unknown_options(context, TRUE);
  g_option_context_add_group(context, gtk_get_option_group(TRUE));

  return context;
}

int main(int argc, char *argv[]) {
  GError *error = NULL;
  GOptionContext *context;
  LogviewApp *app;

#ifdef ENABLE_NLS
  bindtextdomain(GETTEXT_PACKAGE, MATELOCALEDIR);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);
#endif /* ENABLE_NLS */

  context = create_option_context();

  g_option_context_parse(context, &argc, &argv, &error);

  if (error) {
    g_critical("Unable to parse arguments: %s", error->message);
    g_error_free(error);
    g_option_context_free(context);

    exit(1);
  }

  g_option_context_free(context);
  g_set_application_name(_("Log Viewer"));

  app = logview_app_get();

  if (!app) {
    g_critical("Unable to create the user interface.");

    exit(1);
  } else {
    g_signal_connect(app, "app-quit", G_CALLBACK(app_quit_cb), NULL);
  }

  logview_app_initialize(app, log_files);

  gtk_main();

  g_object_unref(app);

  return EXIT_SUCCESS;
}
