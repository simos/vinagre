/*
 * vinagre-utils.c
 * This file is part of vinagre
 *
 * Copyright (C) 2007,2008 - Jonh Wendell <wendell@bani.com.br>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <glib/gi18n.h>
#include "vinagre-utils.h"

#define VINAGRE_GLADE_FILE  "vinagre.glade"
#define VINAGRE_UI_XML_FILE "vinagre-ui.xml"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* For the workspace/viewport stuff */
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#endif

GtkWidget *
vinagre_utils_create_small_close_button ()
{
  GtkRcStyle *rcstyle;
  GtkWidget *image;
  GtkWidget *close_button;

  close_button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (close_button),
			 GTK_RELIEF_NONE);
  /* don't allow focus on the close button */
  gtk_button_set_focus_on_click (GTK_BUTTON (close_button), FALSE);

  /* make it as small as possible */
  rcstyle = gtk_rc_style_new ();
  rcstyle->xthickness = rcstyle->ythickness = 0;
  gtk_widget_modify_style (close_button, rcstyle);
  g_object_unref (rcstyle),

  image = gtk_image_new_from_stock (GTK_STOCK_CLOSE,
				    GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (close_button), image);

  return close_button;
}

void
vinagre_utils_show_error (const gchar *title, const gchar *message, GtkWindow *parent)
{
  GtkWidget *d;
  gchar     *t;

  if (title)
    t = g_strdup (title);
  else
    t = g_strdup (_("An error has occurred:"));

  d = gtk_message_dialog_new (parent,
			      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			      GTK_MESSAGE_ERROR,
			      GTK_BUTTONS_CLOSE,
			      "%s",
			      t);
  g_free (t);

  if (message)
    gtk_message_dialog_format_secondary_markup (GTK_MESSAGE_DIALOG (d),
					      "%s",
					      message);

  g_signal_connect_swapped (d,
			    "response", 
			    G_CALLBACK (gtk_widget_destroy),
			    d);
  gtk_widget_show_all (GTK_WIDGET(d));
}

void
vinagre_utils_show_many_errors (const gchar *title, GSList *items, GtkWindow *parent)
{
  GString *msg;
  GSList  *l;

  msg = g_string_new (NULL);

  for (l = items; l; l = l->next)
    g_string_append_printf (msg, "%s\n", (gchar *)l->data);

  vinagre_utils_show_error (title, msg->str, parent);
  g_string_free (msg, TRUE);
}

void
vinagre_utils_toggle_widget_visible (GtkWidget *widget)
{
  if (GTK_WIDGET_VISIBLE (widget))
    gtk_widget_hide (widget);
  else
    gtk_widget_show_all (widget);
}

const gchar *
vinagre_utils_get_glade_filename (void)
{
  if (g_file_test (VINAGRE_GLADE_FILE, G_FILE_TEST_EXISTS))
    return VINAGRE_GLADE_FILE;
  else
    return VINAGRE_DATADIR "/" VINAGRE_GLADE_FILE;
}

const gchar *
vinagre_utils_get_ui_xml_filename (void)
{
  if (g_file_test (VINAGRE_UI_XML_FILE, G_FILE_TEST_EXISTS))
    return VINAGRE_UI_XML_FILE;
  else
    return VINAGRE_DATADIR "/" VINAGRE_UI_XML_FILE;
}

/*
 * Doubles underscore to avoid spurious menu accels.
 */
gchar * 
vinagre_utils_escape_underscores (const gchar* text,
				  gssize       length)
{
	GString *str;
	const gchar *p;
	const gchar *end;

	g_return_val_if_fail (text != NULL, NULL);

	if (length < 0)
		length = strlen (text);

	str = g_string_sized_new (length);

	p = text;
	end = text + length;

	while (p != end)
	{
		const gchar *next;
		next = g_utf8_next_char (p);

		switch (*p)
		{
			case '_':
				g_string_append (str, "__");
				break;
			default:
				g_string_append_len (str, p, next - p);
				break;
		}

		p = next;
	}

	return g_string_free (str, FALSE);
}

static void _default_log (const gchar *log_domain G_GNUC_UNUSED,
			 GLogLevelFlags log_level G_GNUC_UNUSED,
			 const gchar *message,
			 gpointer user_data G_GNUC_UNUSED)
{
  printf ("gtk-vnc: %s", message);
}

void
vinagre_utils_handle_debug (void)
{
  static gboolean initialized = FALSE;

  if (initialized)
    return;

  g_log_set_handler ("gtk-vnc",
		     G_LOG_LEVEL_DEBUG,
		     _default_log,
		     NULL);

  initialized = TRUE;
}

/* the following two functions are courtesy of galeon */

/**
 * gedit_utils_get_current_workspace: Get the current workspace
 *
 * Get the currently visible workspace for the #GdkScreen.
 *
 * If the X11 window property isn't found, 0 (the first workspace)
 * is returned.
 */
guint
vinagre_utils_get_current_workspace (GdkScreen *screen)
{
#ifdef GDK_WINDOWING_X11
	GdkWindow *root_win;
	GdkDisplay *display;
	Atom type;
	gint format;
	gulong nitems;
	gulong bytes_after;
	guint *current_desktop;
	gint err, result;
	guint ret = 0;

	g_return_val_if_fail (GDK_IS_SCREEN (screen), 0);

	root_win = gdk_screen_get_root_window (screen);
	display = gdk_screen_get_display (screen);

	gdk_error_trap_push ();
	result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (display), GDK_WINDOW_XID (root_win),
				     gdk_x11_get_xatom_by_name_for_display (display, "_NET_CURRENT_DESKTOP"),
				     0, G_MAXLONG, False, XA_CARDINAL, &type, &format, &nitems,
				     &bytes_after, (gpointer) &current_desktop);
	err = gdk_error_trap_pop ();

	if (err != Success || result != Success)
		return ret;

	if (type == XA_CARDINAL && format == 32 && nitems > 0)
		ret = current_desktop[0];

	XFree (current_desktop);
	return ret;
#else
	/* FIXME: on mac etc proably there are native APIs
	 * to get the current workspace etc */
	return 0;
#endif
}

/**
 * gedit_utils_get_window_workspace: Get the workspace the window is on
 *
 * This function gets the workspace that the #GtkWindow is visible on,
 * it returns GEDIT_ALL_WORKSPACES if the window is sticky, or if
 * the window manager doesn support this function
 */
guint
vinagre_utils_get_window_workspace (GtkWindow *gtkwindow)
{
#ifdef GDK_WINDOWING_X11
	GdkWindow *window;
	GdkDisplay *display;
	Atom type;
	gint format;
	gulong nitems;
	gulong bytes_after;
	guint *workspace;
	gint err, result;
	guint ret = VINAGRE_ALL_WORKSPACES;

	g_return_val_if_fail (GTK_IS_WINDOW (gtkwindow), 0);
	g_return_val_if_fail (GTK_WIDGET_REALIZED (GTK_WIDGET (gtkwindow)), 0);

	window = GTK_WIDGET (gtkwindow)->window;
	display = gdk_drawable_get_display (window);

	gdk_error_trap_push ();
	result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (display), GDK_WINDOW_XID (window),
				     gdk_x11_get_xatom_by_name_for_display (display, "_NET_WM_DESKTOP"),
				     0, G_MAXLONG, False, XA_CARDINAL, &type, &format, &nitems,
				     &bytes_after, (gpointer) &workspace);
	err = gdk_error_trap_pop ();

	if (err != Success || result != Success)
		return ret;

	if (type == XA_CARDINAL && format == 32 && nitems > 0)
		ret = workspace[0];

	XFree (workspace);
	return ret;
#else
	/* FIXME: on mac etc proably there are native APIs
	 * to get the current workspace etc */
	return 0;
#endif
}

/**
 * gedit_utils_get_current_viewport: Get the current viewport origin
 *
 * Get the currently visible viewport origin for the #GdkScreen.
 *
 * If the X11 window property isn't found, (0, 0) is returned.
 */
void
vinagre_utils_get_current_viewport (GdkScreen    *screen,
				  gint         *x,
				  gint         *y)
{
#ifdef GDK_WINDOWING_X11
	GdkWindow *root_win;
	GdkDisplay *display;
	Atom type;
	gint format;
	gulong nitems;
	gulong bytes_after;
	gulong *coordinates;
	gint err, result;

	g_return_if_fail (GDK_IS_SCREEN (screen));
	g_return_if_fail (x != NULL && y != NULL);

	/* Default values for the viewport origin */
	*x = 0;
	*y = 0;

	root_win = gdk_screen_get_root_window (screen);
	display = gdk_screen_get_display (screen);

	gdk_error_trap_push ();
	result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (display), GDK_WINDOW_XID (root_win),
				     gdk_x11_get_xatom_by_name_for_display (display, "_NET_DESKTOP_VIEWPORT"),
				     0, G_MAXLONG, False, XA_CARDINAL, &type, &format, &nitems,
				     &bytes_after, (void*) &coordinates);
	err = gdk_error_trap_pop ();

	if (err != Success || result != Success)
		return;

	if (type != XA_CARDINAL || format != 32 || nitems < 2)
	{
		XFree (coordinates);
		return;
	}

	*x = coordinates[0];
	*y = coordinates[1];
	XFree (coordinates);
#else
	/* FIXME: on mac etc proably there are native APIs
	 * to get the current workspace etc */
	*x = 0;
	*y = 0;
#endif
}

/* Make url in about dialog clickable */
static void
vinagre_about_dialog_handle_url (GtkAboutDialog *about,
				 const char     *link,
				 gpointer        data)
{
  GError    *error = NULL;
  gchar     *address;
  GdkScreen *screen;

  if (g_strstr_len (link, strlen (link), "@"))
    address = g_strdup_printf ("mailto:%s", link);
  else
    address = g_strdup (link);

  screen = GTK_IS_WINDOW (data) ? gtk_window_get_screen (GTK_WINDOW (data)) : NULL;

  gtk_show_uri (screen,
		address,
		GDK_CURRENT_TIME,
		&error);

  if (error != NULL) 
    {
      vinagre_utils_show_error (NULL, error->message, GTK_IS_WINDOW (data) ? GTK_WINDOW (data) : NULL);
      g_error_free (error);
    }

  g_free (address);
}

void
vinagre_utils_help_contents (GtkWindow *window)
{
  GError    *error;
  GdkScreen *screen;

  screen = GTK_IS_WINDOW (window) ? gtk_window_get_screen (GTK_WINDOW (window)) : NULL;
  error = NULL;

  gtk_show_uri (screen,
		"ghelp:vinagre",
		GDK_CURRENT_TIME,
		&error);

  if (error != NULL) 
    {
      vinagre_utils_show_error (NULL, error->message, GTK_IS_WINDOW (window) ? window : NULL);
      g_error_free (error);
    }
}

void
vinagre_utils_help_about (GtkWindow *window)
{
  static const gchar * const authors[] = {
	"Jonh Wendell <jwendell@gnome.org>",
	NULL
  };

  static const gchar * const artists[] = {
	"Vinicius Depizzol <vdepizzol@gmail.com>",
	NULL
  };

  static const gchar copyright[] = \
	"Copyright \xc2\xa9 2007 Jonh Wendell";

  static const gchar comments[] = \
	N_("Vinagre is a VNC client for the GNOME Desktop");

  static const char *license[] = {
	N_("Vinagre is free software; you can redistribute it and/or modify "
	   "it under the terms of the GNU General Public License as published by "
	   "the Free Software Foundation; either version 2 of the License, or "
	   "(at your option) any later version."),
	N_("Vinagre is distributed in the hope that it will be useful, "
	   "but WITHOUT ANY WARRANTY; without even the implied warranty of "
	   "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
	   "GNU General Public License for more details."),
	N_("You should have received a copy of the GNU General Public License "
	   "along with this program. If not, see <http://www.gnu.org/licenses/>.")
  };

  gchar *license_trans;

  license_trans = g_strjoin ("\n\n", _(license[0]), _(license[1]),
				     _(license[2]), NULL);

  /* Make URLs and email clickable in about dialog */
  gtk_about_dialog_set_url_hook (vinagre_about_dialog_handle_url, window, NULL);
  gtk_about_dialog_set_email_hook (vinagre_about_dialog_handle_url, window, NULL);


  gtk_show_about_dialog (GTK_IS_WINDOW (window)?window:NULL,
			 "authors", authors,
			 "artists", artists,
			 "comments", _(comments),
			 "copyright", copyright,
			 "license", license_trans,
			 "wrap-license", TRUE,
			 "logo-icon-name", "vinagre",
			 "translator-credits", _("translator-credits"),
			 "version", VERSION,
			 "website", "http://projects.gnome.org/vinagre/",
			 "website-label", _("Vinagre Website"),
			 NULL);
  g_free (license_trans);
}

/* vim: set ts=8: */
