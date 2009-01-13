/*
 * vinagre-mdns.h
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

#ifndef _VINAGRE_MDNS_H_
#define _VINAGRE_MDNS_H_

#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

#define VINAGRE_TYPE_MDNS             (vinagre_mdns_get_type ())
#define VINAGRE_MDNS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), VINAGRE_TYPE_MDNS, VinagreMdns))
#define VINAGRE_MDNS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), VINAGRE_TYPE_MDNS, VinagreMdnsClass))
#define VINAGRE_IS_MDNS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VINAGRE_TYPE_MDNS))
#define VINAGRE_IS_MDNS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_MDNS))
#define VINAGRE_MDNS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), VINAGRE_TYPE_MDNS, VinagreMdnsClass))

typedef struct _VinagreMdnsClass   VinagreMdnsClass;
typedef struct _VinagreMdns        VinagreMdns;
typedef struct _VinagreMdnsPrivate VinagreMdnsPrivate;

struct _VinagreMdnsClass
{
  GObjectClass parent_class;

  /* Signals */
  void (* changed) (VinagreMdns *mdns);
};

struct _VinagreMdns
{
  GObject parent_instance;
  VinagreMdnsPrivate *priv;
};

GType        vinagre_mdns_get_type    (void) G_GNUC_CONST;

VinagreMdns *vinagre_mdns_get_default (void);
GSList      *vinagre_mdns_get_all     (VinagreMdns *mdns);

G_END_DECLS

#endif /* _VINAGRE_MDNS_H_ */
/* vim: set ts=8: */
