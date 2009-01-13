/*
 * vinagre-mdns.c
 * This file is part of vinagre
 *
 * Copyright (C) Jonh Wendell 2008 <wendell@bani.com.br>
 * 
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "vinagre-mdns.h"
#include "vinagre-connection.h"
#include "vinagre-bookmarks-entry.h"
#include <avahi-gobject/ga-service-browser.h>
#include <avahi-gobject/ga-service-resolver.h>
#include <glib/gi18n.h>

struct _VinagreMdnsPrivate
{
  GSList           *entries;
  GaServiceBrowser *browser;
  GaClient         *client;
};

enum
{
  MDNS_CHANGED,
  LAST_SIGNAL
};

G_DEFINE_TYPE (VinagreMdns, vinagre_mdns, G_TYPE_OBJECT);

static VinagreMdns *mdns_singleton = NULL;
static guint signals[LAST_SIGNAL] = { 0 };

static void
mdns_resolver_found (GaServiceResolver *resolver,
                     AvahiIfIndex         iface,
                     GaProtocol           proto,
                     gchar               *name,
                     gchar               *type,
                     gchar               *domain,
                     gchar               *host_name,
                     AvahiAddress        *a,
                     gint                 port,
                     AvahiStringList     *txt,
                     GaLookupResultFlags flags,
                     VinagreMdns         *mdns)
{
  VinagreConnection     *conn;
  VinagreBookmarksEntry *entry;

  conn = vinagre_connection_new ();
  g_object_set (conn,
                "name", name,
                "port", port,
                "host", host_name,
                NULL);
  entry = vinagre_bookmarks_entry_new_conn (conn);
  g_object_unref (conn);

  mdns->priv->entries = g_slist_insert_sorted (mdns->priv->entries,
					       entry,
					       (GCompareFunc)vinagre_bookmarks_entry_compare);

  g_object_unref (resolver);
  g_signal_emit (mdns, signals[MDNS_CHANGED], 0);
}

static void
mdns_resolver_failure (GaServiceResolver *resolver,
                       GError            *error,
                       VinagreMdns       *mdns)
{
  g_warning ("%s", error->message);
  g_object_unref (resolver);
}

static void
mdns_browser_new_cb (GaServiceBrowser   *browser,
                     AvahiIfIndex        iface,
                     GaProtocol          proto,
                     gchar              *name,
                     gchar              *type,
                     gchar              *domain,
                     GaLookupResultFlags flags,
                     VinagreMdns        *mdns)
{
  GaServiceResolver *resolver;
  GError *error = NULL;

  resolver = ga_service_resolver_new (iface,
                                      proto,
                                      name,
                                      type,
                                      domain,
                                      GA_PROTOCOL_UNSPEC,
                                      GA_LOOKUP_NO_FLAGS);

  g_signal_connect (resolver,
                    "found",
                    G_CALLBACK (mdns_resolver_found),
                    mdns);
  g_signal_connect (resolver,
                    "failure",
                    G_CALLBACK (mdns_resolver_failure),
                    mdns);

  if (!ga_service_resolver_attach (resolver,
                                   mdns->priv->client,
                                   &error))
    {
      g_warning (_("Failed to resolve avahi hostname: %s\n"), error->message);
      g_error_free (error);
    }
}

static void
mdns_browser_del_cb (GaServiceBrowser   *browser,
                     AvahiIfIndex        iface,
                     GaProtocol          proto,
                     gchar              *name,
                     gchar              *type,
                     gchar              *domain,
                     GaLookupResultFlags flags,
                     VinagreMdns        *mdns)
{
  GSList *l;

  for (l = mdns->priv->entries; l; l = l->next)
    {
      VinagreBookmarksEntry *entry = VINAGRE_BOOKMARKS_ENTRY (l->data);
      if (strcmp (vinagre_connection_get_name (vinagre_bookmarks_entry_get_conn (entry)), name) == 0)
	{
	  mdns->priv->entries = g_slist_remove (mdns->priv->entries, entry);
	  g_object_unref (entry);
	  g_signal_emit (mdns, signals[MDNS_CHANGED], 0);
	  return;
	}
    }
}

static void
vinagre_mdns_init (VinagreMdns *mdns)
{
  GError *error = NULL;

  mdns->priv = G_TYPE_INSTANCE_GET_PRIVATE (mdns, VINAGRE_TYPE_MDNS, VinagreMdnsPrivate);

  mdns->priv->entries = NULL;
  mdns->priv->browser = ga_service_browser_new ("_rfb._tcp");
  mdns->priv->client = ga_client_new (GA_CLIENT_FLAG_NO_FLAGS);

  g_signal_connect (mdns->priv->browser,
                    "new-service",
                    G_CALLBACK (mdns_browser_new_cb),
                    mdns);
  g_signal_connect (mdns->priv->browser,
                    "removed-service",
                    G_CALLBACK (mdns_browser_del_cb),
                    mdns);

  if (!ga_client_start (mdns->priv->client, &error))
    {
        g_warning (_("Failed to browse for hosts: %s\n"), error->message);
        g_error_free (error);
        return;
    }

  if (!ga_service_browser_attach (mdns->priv->browser,
                                  mdns->priv->client,
                                  &error))
    {
        g_warning (_("Failed to browse for hosts: %s\n"), error->message);
        g_error_free (error);
    }
}

static void
vinagre_mdns_dispose (GObject *object)
{
  VinagreMdns *mdns = VINAGRE_MDNS (object);

  if (mdns->priv->browser)
    {
      g_object_unref (mdns->priv->browser);
      mdns->priv->browser = NULL;
    }

  if (mdns->priv->client)
    {
      g_object_unref (mdns->priv->client);
      mdns->priv->client = NULL;
    }

  if (mdns->priv->entries)
    {
      g_slist_foreach (mdns->priv->entries, (GFunc) g_object_unref, NULL);
      g_slist_free (mdns->priv->entries);
      mdns->priv->entries = NULL;
    }

  G_OBJECT_CLASS (vinagre_mdns_parent_class)->dispose (object);
}

static void
vinagre_mdns_class_init (VinagreMdnsClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  GObjectClass* parent_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (VinagreMdnsPrivate));

  object_class->dispose = vinagre_mdns_dispose;

  signals[MDNS_CHANGED] =
		g_signal_new ("changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (VinagreMdnsClass, changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

}

VinagreMdns *
vinagre_mdns_get_default (void)
{
  if (G_UNLIKELY (!mdns_singleton))
    mdns_singleton = VINAGRE_MDNS (g_object_new (VINAGRE_TYPE_MDNS,
                                                 NULL));
  return mdns_singleton;
}

GSList *
vinagre_mdns_get_all (VinagreMdns *mdns)
{
  g_return_val_if_fail (VINAGRE_IS_MDNS (mdns), NULL);

  return mdns->priv->entries;
}
