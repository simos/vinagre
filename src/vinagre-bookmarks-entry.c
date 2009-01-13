/*
 * vinagre-bookmarks-entry.c
 * This file is part of vinagre
 *
 * Copyright (C) 2008  Jonh Wendell <wendell@bani.com.br>
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

#include "vinagre-bookmarks-entry.h"

struct _VinagreBookmarksEntryPrivate
{
  VinagreBookmarksEntryNode node;
  VinagreConnection        *conn;
  gchar                    *name;
  GSList                   *children; // array of VinagreBookmarksEntry
  VinagreBookmarksEntry    *parent;
};

G_DEFINE_TYPE (VinagreBookmarksEntry, vinagre_bookmarks_entry, G_TYPE_OBJECT);

static void
vinagre_bookmarks_entry_init (VinagreBookmarksEntry *entry)
{
  entry->priv = G_TYPE_INSTANCE_GET_PRIVATE (entry, VINAGRE_TYPE_BOOKMARKS_ENTRY, VinagreBookmarksEntryPrivate);

  entry->priv->node = VINAGRE_BOOKMARKS_ENTRY_NODE_INVALID;
  entry->priv->conn = NULL;
  entry->priv->name = NULL;
  entry->priv->children = NULL;
  entry->priv->parent = NULL;
}

static void
vinagre_bookmarks_entry_finalize (GObject *object)
{
  VinagreBookmarksEntry *entry = VINAGRE_BOOKMARKS_ENTRY (object);

  if (entry->priv->node == VINAGRE_BOOKMARKS_ENTRY_NODE_FOLDER)
    g_free (entry->priv->name);

  G_OBJECT_CLASS (vinagre_bookmarks_entry_parent_class)->finalize (object);
}

static void
vinagre_bookmarks_entry_dispose (GObject *object)
{
  VinagreBookmarksEntry *entry = VINAGRE_BOOKMARKS_ENTRY (object);

  switch (entry->priv->node)
    {
      case VINAGRE_BOOKMARKS_ENTRY_NODE_CONN:
	if (entry->priv->conn)
	  {
	    g_object_unref (entry->priv->conn);
	    entry->priv->conn = NULL;
	  }
	break;

      case VINAGRE_BOOKMARKS_ENTRY_NODE_FOLDER:
	if (entry->priv->children)
	  {
	    g_slist_foreach (entry->priv->children, (GFunc) g_object_unref, NULL);
	    g_slist_free (entry->priv->children);
	    entry->priv->children = NULL;
	  }
	break;

      default:
	g_assert_not_reached ();
    }

  G_OBJECT_CLASS (vinagre_bookmarks_entry_parent_class)->dispose (object);
}

static void
vinagre_bookmarks_entry_class_init (VinagreBookmarksEntryClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (VinagreBookmarksEntryPrivate));

  object_class->finalize = vinagre_bookmarks_entry_finalize;
  object_class->dispose  = vinagre_bookmarks_entry_dispose;
}

VinagreBookmarksEntry *
vinagre_bookmarks_entry_new_folder (const gchar *name)
{
  VinagreBookmarksEntry *entry;

  g_return_val_if_fail (name != NULL, NULL);

  entry = VINAGRE_BOOKMARKS_ENTRY (g_object_new (VINAGRE_TYPE_BOOKMARKS_ENTRY, NULL));
  vinagre_bookmarks_entry_set_node (entry, VINAGRE_BOOKMARKS_ENTRY_NODE_FOLDER);
  vinagre_bookmarks_entry_set_name (entry, name);

  return entry;
}

VinagreBookmarksEntry *
vinagre_bookmarks_entry_new_conn (VinagreConnection *conn)
{
  VinagreBookmarksEntry *entry;

  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), NULL);

  entry = VINAGRE_BOOKMARKS_ENTRY (g_object_new (VINAGRE_TYPE_BOOKMARKS_ENTRY, NULL));
  vinagre_bookmarks_entry_set_node (entry, VINAGRE_BOOKMARKS_ENTRY_NODE_CONN);
  vinagre_bookmarks_entry_set_conn (entry, conn);

  return entry;

}

void
vinagre_bookmarks_entry_set_node (VinagreBookmarksEntry *entry, VinagreBookmarksEntryNode node)
{
  g_return_if_fail (VINAGRE_IS_BOOKMARKS_ENTRY (entry));

  if (entry->priv->node == node)
    return;

  entry->priv->node = node;

  switch (entry->priv->node)
    {
      case VINAGRE_BOOKMARKS_ENTRY_NODE_CONN:
	g_free (entry->priv->name);
	entry->priv->name = NULL;
	g_slist_foreach (entry->priv->children, (GFunc) g_object_unref, NULL);
	g_slist_free (entry->priv->children);
	entry->priv->children = NULL;
	break;

      case VINAGRE_BOOKMARKS_ENTRY_NODE_FOLDER:
	if (entry->priv->conn)
	  {
	    g_object_unref (entry->priv->conn);
	    entry->priv->conn = NULL;
	  }
	break;

      default:
	g_assert_not_reached ();
    }
}

VinagreBookmarksEntryNode
vinagre_bookmarks_entry_get_node (VinagreBookmarksEntry *entry)
{
  g_return_val_if_fail (VINAGRE_IS_BOOKMARKS_ENTRY (entry), VINAGRE_BOOKMARKS_ENTRY_NODE_INVALID);

  return entry->priv->node;
}

void
vinagre_bookmarks_entry_set_conn (VinagreBookmarksEntry *entry, VinagreConnection *conn)
{
  g_return_if_fail (VINAGRE_IS_BOOKMARKS_ENTRY (entry));
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));
  g_return_if_fail (entry->priv->node == VINAGRE_BOOKMARKS_ENTRY_NODE_CONN);

  if (entry->priv->conn != NULL)
    g_object_unref (entry->priv->conn);

  entry->priv->conn = g_object_ref (conn);
}

VinagreConnection *
vinagre_bookmarks_entry_get_conn (VinagreBookmarksEntry *entry)
{
  g_return_val_if_fail (VINAGRE_IS_BOOKMARKS_ENTRY (entry), NULL);

  return entry->priv->conn;
}

void
vinagre_bookmarks_entry_set_name (VinagreBookmarksEntry *entry, const gchar *name)
{
  g_return_if_fail (VINAGRE_IS_BOOKMARKS_ENTRY (entry));
  g_return_if_fail (entry->priv->node == VINAGRE_BOOKMARKS_ENTRY_NODE_FOLDER);
  g_return_if_fail (name != NULL);

  g_free (entry->priv->name);
  entry->priv->name = g_strdup (name);
}

const gchar *
vinagre_bookmarks_entry_get_name (VinagreBookmarksEntry *entry)
{
  g_return_val_if_fail (VINAGRE_IS_BOOKMARKS_ENTRY (entry), NULL);

  return entry->priv->name;
}

void
vinagre_bookmarks_entry_add_child (VinagreBookmarksEntry *entry, VinagreBookmarksEntry *child)
{
  g_return_if_fail (VINAGRE_IS_BOOKMARKS_ENTRY (entry));
  g_return_if_fail (entry->priv->node == VINAGRE_BOOKMARKS_ENTRY_NODE_FOLDER);
  g_return_if_fail (VINAGRE_IS_BOOKMARKS_ENTRY (child));

  entry->priv->children = g_slist_insert_sorted (entry->priv->children,
						 child,
						 (GCompareFunc)vinagre_bookmarks_entry_compare);
  child->priv->parent = entry;
}

gboolean
vinagre_bookmarks_entry_remove_child (VinagreBookmarksEntry *entry,
				      VinagreBookmarksEntry *child)
{
  GSList *l;

  g_return_val_if_fail (VINAGRE_IS_BOOKMARKS_ENTRY (entry), FALSE);
  g_return_val_if_fail (VINAGRE_IS_BOOKMARKS_ENTRY (child), FALSE);

  if (g_slist_index (entry->priv->children, child) > -1)
    {
      entry->priv->children = g_slist_remove (entry->priv->children, child);
      return TRUE;
    }

  for (l = entry->priv->children; l; l = l->next)
    {
      VinagreBookmarksEntry *e = (VinagreBookmarksEntry *) l->data;

      if (vinagre_bookmarks_entry_get_node (e) != VINAGRE_BOOKMARKS_ENTRY_NODE_FOLDER)
	continue;

      if (vinagre_bookmarks_entry_remove_child (e, child))
	return TRUE;
    }

  return FALSE;
}

GSList *
vinagre_bookmarks_entry_get_children (VinagreBookmarksEntry *entry)
{
  g_return_val_if_fail (VINAGRE_IS_BOOKMARKS_ENTRY (entry), NULL);

  return entry->priv->children;
}

VinagreBookmarksEntry *
vinagre_bookmarks_entry_get_parent (VinagreBookmarksEntry *entry)
{
  g_return_val_if_fail (VINAGRE_IS_BOOKMARKS_ENTRY (entry), NULL);

  return entry->priv->parent;
}

gint
vinagre_bookmarks_entry_compare (VinagreBookmarksEntry *a, VinagreBookmarksEntry *b)
{
  gchar *name_a, *name_b;
  gint   result;

  if (a->priv->node != b->priv->node)
    return a->priv->node - b->priv->node;

  if (a->priv->node == VINAGRE_BOOKMARKS_ENTRY_NODE_FOLDER)
    return g_ascii_strcasecmp (a->priv->name, b->priv->name);

  name_a = vinagre_connection_get_best_name (a->priv->conn);
  name_b = vinagre_connection_get_best_name (b->priv->conn);
  result = g_ascii_strcasecmp (name_a, name_b);
  g_free (name_a);
  g_free (name_b);

  return result;
}

/* vim: set ts=8: */
