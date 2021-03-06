SUBDIRS = view

NULL = 

##############################
# The main binary
##############################

INCLUDES = 						\
	-DBINDIR=\"$(bindir)\"			 	\
	-DDATADIR=\"$(datadir)\"			\
	-DPREFIX=\""$(prefix)"\" 			\
	-DSYSCONFDIR=\""$(sysconfdir)"\" 		\
	-DLIBDIR=\""$(libdir)"\" 			\
	-DVINAGRE_DATADIR=\""$(datadir)/vinagre"\"	\
	-DPACKAGE_LOCALE_DIR=\""$(datadir)/locale"\"	\
	$(VINAGRE_CFLAGS)				\
	$(AVAHI_CFLAGS)					\
	$(NULL)

bin_PROGRAMS = vinagre

vinagre_SOURCES = \
	vinagre-commands.c vinagre-commands.h			\
	vinagre-connect.c vinagre-connect.h			\
	vinagre-connection.c vinagre-connection.h		\
	vinagre-fav.c vinagre-fav.h				\
	vinagre-bookmarks.c vinagre-bookmarks.h			\
	vinagre-main.c 						\
	vinagre-notebook.c vinagre-notebook.h			\
	vinagre-tab.c vinagre-tab.h				\
	vinagre-utils.c vinagre-utils.h				\
	vinagre-window.c vinagre-window.h			\
	vinagre-ui.h vinagre-window-private.h			\
	vinagre-enums.h vinagre-enums.c				\
	vinagre-prefs.h vinagre-prefs.c				\
	bacon-message-connection.h bacon-message-connection.c	\
	vinagre-app.h vinagre-app.c 				\
	vinagre-bacon.h vinagre-bacon.c 			\
	vinagre-bookmarks-entry.h vinagre-bookmarks-entry.c	\
	vinagre-bookmarks-tree.h vinagre-bookmarks-tree.c	\
	vinagre-bookmarks-ui.h vinagre-bookmarks-ui.c		\
	vinagre-bookmarks-migration.h vinagre-bookmarks-migration.c \
	$(NULL)

if AVAHI
vinagre_SOURCES += vinagre-mdns.h vinagre-mdns.c
endif

vinagre_LDADD = \
	$(VINAGRE_LIBS)		\
	$(AVAHI_LIBS)		\
	$(GTKVNC_LIBS)		\
	view/libview.la		\
	$(NULL)

##############################
# Bonobo .server
##############################

serverdir =					\
	$(libdir)/bonobo/servers
server_in_files =				\
	GNOME_VinagreApplet.server.in
server_DATA =					\
	$(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" -e "s|\@VERSION\@|$(VERSION)|" $< > $@

@INTLTOOL_SERVER_RULE@

##############################
# The applet binary
##############################

libexec_PROGRAMS = vinagre-applet

vinagre_applet_CFLAGS =			\
	$(APPLET_CFLAGS)		\
	$(AVAHI_CFLAGS)			\
	$(NULL)

vinagre_applet_SOURCES =					\
	vinagre-applet.c					\
	vinagre-bookmarks.h vinagre-bookmarks.c			\
	vinagre-connection.h vinagre-connection.c		\
	vinagre-utils.h vinagre-utils.c				\
	vinagre-enums.h vinagre-enums.c				\
	vinagre-bookmarks-entry.h vinagre-bookmarks-entry.c	\
	vinagre-bookmarks-tree.h vinagre-bookmarks-tree.c	\
	vinagre-bookmarks-migration.h vinagre-bookmarks-migration.c \
	$(NULL)

if AVAHI
vinagre_applet_SOURCES += vinagre-mdns.h vinagre-mdns.c
endif

vinagre_applet_LDADD =			\
	$(APPLET_LIBS)			\
	$(VINAGRE_LIBS)			\
	$(AVAHI_LIBS)			\
	$(NULL)


##############################
# Autogenerated stuff
##############################

vinagre_enum_headers = \
	$(top_srcdir)/src/vinagre-connection.h \
	$(NULL)

vinagre-enums.c: @REBUILD@ $(vinagre_enum_headers)
	glib-mkenums    --fhead "#include <glib-object.h>\n" \
			--fhead "#include \"vinagre-enums.h\"\n\n" \
			--fprod "\n/* enumerations from \"@filename@\" */" \
			--fprod "\n#include \"@filename@\"\n" \
			--vhead "static const G@Type@Value _@enum_name@_values[] = {" \
			--vprod "  { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
			--vtail "  { 0, NULL, NULL }\n};\n\n" \
			--vtail "GType\n@enum_name@_get_type (void)\n{\n" \
			--vtail "  static GType type = 0;\n\n" \
			--vtail "  if (!type)\n" \
			--vtail "    type = g_@type@_register_static (\"@EnumName@\", _@enum_name@_values);\n\n" \
			--vtail "  return type;\n}\n\n" \
		$(vinagre_enum_headers) > $@

vinagre-enums.h: @REBUILD@ $(vinagre_enum_headers)
	glib-mkenums    --fhead "#ifndef __VINAGRE_ENUMS_H__\n" \
			--fhead "#define __VINAGRE_ENUMS_H__ 1\n\n" \
			--fhead "G_BEGIN_DECLS\n\n" \
			--ftail "G_END_DECLS\n\n" \
			--ftail "#endif /* __VINAGRE_ENUMS_H__ */\n" \
			--fprod "\n/* --- @filename@ --- */" \
			--eprod "#define VINAGRE_TYPE_@ENUMSHORT@ @enum_name@_get_type()\n" \
			--eprod "GType @enum_name@_get_type (void);\n" \
		$(vinagre_enum_headers) >  $@

BUILT_SOURCES = \
	vinagre-enums.c \
	vinagre-enums.h \
	$(NULL)

DISTCLEANFILES =		\
	$(BUILT_SOURCES)	\
	$(server_in_files)	\
	$(server_DATA)		\
	$(NULL)

EXTRA_DIST =					\
	GNOME_VinagreApplet.server.in.in	\
	$(NULL)

dist-hook:
	cd $(distdir) ; rm -f $(DISTCLEANFILES)

$(vinagre_OBJECTS): $(BUILT_SOURCES)
