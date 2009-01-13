SUBDIRS = src help po data

vinagredocdir = ${datadir}/doc/vinagre
vinagredoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS\
	MAINTAINERS

EXTRA_DIST =			\
	$(vinagredoc_DATA)	\
	intltool-extract.in	\
	intltool-merge.in	\
	intltool-update.in

DISTCLEANFILES =		\
	intltool-extract	\
	intltool-merge		\
	intltool-update

DISTCHECK_CONFIGURE_FLAGS = --disable-scrollkeeper

# Copy all the spec files. Of cource, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done

