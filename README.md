mate-utils
=========================
Contains MATE Utility programs for the MATE Desktop, namely the following:

* mate-system-log          [logview]
* mate-search-tool         [gsearchtool]
* mate-dictionary          [mate-dictionary]
* mate-screenshot          [mate-screenshot]
* mate-disk-usage-analyzer [baobab]

mate-utils is a fork of GNOME Utilities. 
This software is licensed under the GNU GPL. For more on the license, see COPYING.

Requirements
========================
| Pakage | Version |
| :----: | :-----: |
| mate-common | >= 1.24.1 |
| GLib | >= 2.62 |
| GIO | >= 2.62
| GTK+ | >= 3.22
| libmate-panel-applet | >= 1.17
| libgtop | >= 2.12
| libcanberra-gtk | >= 0.4 |

The following configure flags can be used:

-  `--enable-zlib`: Enable ZLib support for Logview [default=yes]

-  `--with-grep`: Specify the path to the grep command [default=find it ourselves]

-  `--enable-debug`: Enable debug messages [default=no]
