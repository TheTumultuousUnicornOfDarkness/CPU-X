#!/usr/bin/sh

mkdir -p embed
gdk-pixbuf-csource --raw --name=AMD	data/AMD.png	> embed/AMD.png.h
gdk-pixbuf-csource --raw --name=Intel	data/Intel.png	> embed/Intel.png.h
gdk-pixbuf-csource --raw --name=CPU_X	data/CPU-X.png	> embed/CPU-X.png.h
echo -e "#ifndef CPUX_GLADE_H\n#define CPUX_GLADE_H\n\nstatic const char *cpux_glade =" > embed/cpu-x.ui.h
sed -e 's/\\/\\\\/g;s/"/\\"/g;s/ / /g;s/^/"/;s/$/\\n"/' 'data/cpux-gtk-3.8.ui' >> embed/cpu-x.ui.h
echo -e ";\n\n#endif\n" >> embed/cpu-x.ui.h
