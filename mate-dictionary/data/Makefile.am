# This file is part of MATE Utils.
#
# MATE Utils is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# MATE Utils is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with MATE Utils.  If not, see <https://www.gnu.org/licenses/>.

NULL =

man_MANS = mate-dictionary.1

gdictappdir = $(datadir)/applications
gdictapp_in_files = mate-dictionary.desktop.in
gdictapp_DATA = $(gdictapp_in_files:.desktop.in=.desktop)

$(gdictapp_in_files): $(gdictapp_in_files:.desktop.in=.desktop.in.in)
	$(AM_V_GEN)$(SED) \
		-e "s|\@VERSION\@|@VERSION@|" \
		$< > $@

$(gdictapp_DATA): $(gdictapp_in_files)
if USE_NLS
	$(AM_V_GEN) $(MSGFMT) --desktop --keyword= --keyword=Name --keyword=Comment --keyword=Keywords --template $< -d $(top_srcdir)/po -o $@
else
	$(AM_V_GEN) cp $< $@
endif

# dictionary sources definitions
dictsourcedir = $(datadir)/mate-dict/sources
dictsource_in_files = \
	default.desktop.in \
	thai.desktop.in \
	$(NULL)
dictsource_DATA = $(dictsource_in_files:.desktop.in=.desktop)

$(dictsource_DATA): $(dictsource_in_files)
if USE_NLS
	$(AM_V_GEN) $(MSGFMT) --desktop --keyword= --keyword=Name --keyword=Description --template $< -d $(top_srcdir)/po -o $@
else
	$(AM_V_GEN) cp $< $@
endif

builderdir = $(datadir)/mate-dictionary
builder_DATA = \
	mate-dictionary-preferences.ui	\
	mate-dictionary-source.ui	\
	$(NULL)

uidir = $(datadir)/mate-dictionary
ui_DATA = mate-dictionary-ui.xml

if BUILD_GDICT_APPLET

# applet file, dbus service, and menu definition
appletdir = $(datadir)/mate-panel/applets
applet_in_files = org.mate.DictionaryApplet.mate-panel-applet.desktop.in
applet_DATA = $(applet_in_files:.mate-panel-applet.desktop.in=.mate-panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)$(SED) \
		-e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
		$< > $@

$(applet_DATA): $(applet_in_files) Makefile
if USE_NLS
	$(AM_V_GEN) $(MSGFMT) --desktop --keyword=Name --keyword=Description --template $< -d $(top_srcdir)/po -o $@
else
	$(AM_V_GEN) cp $< $@
endif

servicedir = $(datadir)/dbus-1/services
service_in_files = org.mate.panel.applet.DictionaryAppletFactory.service.in
service_DATA = $(service_in_files:.service.in=.service)

org.mate.panel.applet.DictionaryAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)$(SED) \
		-e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
		$< > $@

ui_DATA += dictionary-applet-menu.xml

endif # BUILD_GDICT_APPLET

appdatadir = $(datadir)/metainfo
appdata_in_files = mate-dictionary.appdata.xml.in
appdata_DATA = $(appdata_in_files:.xml.in=.xml)

$(appdata_DATA): $(appdata_in_files)
if USE_NLS
	$(AM_V_GEN) $(MSGFMT) --xml --template $< -d $(top_srcdir)/po -o $@
else
	$(AM_V_GEN) cp $< $@
endif

gsettings_SCHEMAS = org.mate.dictionary.gschema.xml
@GSETTINGS_RULES@

EXTRA_DIST = \
	$(applet_in_files).in \
	$(service_in_files) \
	$(gdictapp_in_files).in \
	$(dictsource_in_files) \
	$(man_MANS) \
	$(appdata_in_files) \
	$(builder_DATA) \
	$(ui_DATA) \
	$(NULL)

CLEANFILES = \
	$(applet_DATA) \
	$(applet_DATA).in \
	$(applet_in_files) \
	$(service_DATA) \
	$(dictsource_DATA) \
	$(gdictapp_in_files) \
	$(gdictapp_DATA) \
	$(gsettings_SCHEMAS) \
	mate-dictionary.appdata.xml \
	$(NULL)

-include $(top_srcdir)/git.mk
