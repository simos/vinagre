SUBDIRS = icons

gladedir   = $(datadir)/vinagre
glade_DATA = vinagre.glade

man_MANS = vinagre.1

uidir = $(datadir)/vinagre
ui_DATA = vinagre-ui.xml GNOME_VinagreApplet.xml

@INTLTOOL_DESKTOP_RULE@
desktopdir       = $(datadir)/applications
desktop_in_files = vinagre.desktop.in vinagre-file.desktop.in
desktop_DATA     = $(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_XML_RULE@
MIMEFILES = vinagre-mime.xml.in
mimedir = $(datadir)/mime/packages
mime_DATA = vinagre-mime.xml

schemasdir	 = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = vinagre.schemas.in
schemas_DATA 	 = $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

if GCONF_SCHEMAS_INSTALL
install-data-local:
	if test -z "$(DESTDIR)" ; then \
		for p in $(schemas_DATA) ; do \
			GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/data/$$p ; \
		done \
	fi
else
install-data-local:
endif

CLEANFILES = \
	$(desktop_DATA) \
	$(mime_DATA) \
	$(schemas_DATA)

EXTRA_DIST = \
	$(glade_DATA) \
	$(man_MANS) \
	$(ui_DATA) \
	$(desktop_in_files) \
	$(MIMEFILES) \
	$(schemas_in_files)

UPDATE_MIME = \
        if [ -f $(DESTDIR)$(datadir)/mime/packages/freedesktop.org.xml ] ; then \
                if which update-mime-database>/dev/null 2>&1; then \
                        update-mime-database $(DESTDIR)$(datadir)/mime; \
                fi \
        fi

UPDATE_DESKTOP = \
        if [ -f $(DESTDIR)$(datadir)/applications/defaults.list ] ; then \
                if which update-desktop-database>/dev/null 2>&1 ; then \
                        update-desktop-database; \
                fi \
        fi

install-data-hook: 
	$(UPDATE_MIME)
	$(UPDATE_DESKTOP)

uninstall-hook: 
	$(UPDATE_MIME)
	$(UPDATE_DESKTOP)
