# GEEPRO README FILE

## COMPILATION

The programm geepro uses [waf](https://github.com/waf-project/waf "WAF on Github") build tool.

To compile geepro, do:

`./waf configure`
`./waf build`

To compile and install geepro to some specified location, do:

`./waf configure --prefix=/usr/local`
`./waf build`
`./waf install`

To start cleanly:

`./waf distclean`

## Dependency
I found this dependency list on the Arch Linux GeePro page. I've modified it for the Debian package names. I'd expect similar dependencies for Ubuntu and any other Debian/Ubuntu based Linux Distribution.

* **cairo** (cairo-coloredemoji, cairo-dfb, cairo-git, cairo-infinality, cairo-infinality-lcdfilter, cairo-infinality-ultimate, cairo-infinality-ultimate-with-colored-emoji, cairo-minimal, cairo-nomesa, cairo-ubuntu) Debian - cairo dev
gettext
* **gtk3** (gtk3-aqd, gtk3-cosy, gtk3-donnatella, gtk3-light, gtk3-mushrooms, gtk3-nobroadway-nowayland-nocolord, gtk3-optional-csd, gtk3-typeahead, gtk3-ubuntu, gtk3-ubuntu-multilib) Debian - libgtk-3.0 dev
* **intltool**
* **libxml2** (libxml2-linenum)
* **perl** (perl-fake)
* **pkg-config** (pkgconf-git, pkgconf)

## Issues

* waf - I know little about waf and while it might be a great system I don't have time to learn an entirely new system. It does work very well but I have a lot of other stuff to learn.. So, I'd rather have a Makefile. I'll look into that. I have no intention of deleting it.

## License
GNU GENERAL PUBLIC LICENSE
                       Version 2, June 1991

## History
I've copied down the  0.0.4 GeePro tar ball from SourceForge. I've not changed much but I have updated the Readme file to reflect the dependencies I've found. I've also run ./waf distclean and removed the .svn directory. I'll share the Readme with the existing project so others can find it too.

[GeePro original repos](https://sourceforge.net/projects/geepro/ "GeePro home page on SourceForge")