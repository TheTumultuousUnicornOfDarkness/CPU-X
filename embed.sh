#!/usr/bin/sh

mkdir -p embed
gdk-pixbuf-csource --raw --name=CPU_X			data/pictures/CPU-X.png		> embed/CPU-X.png.h
gdk-pixbuf-csource --raw --name=NOVENDOR		data/pictures/novendor.png	> embed/NoVendor.png.h
gdk-pixbuf-csource --raw --name=AMD			data/pictures/AMD.png		> embed/AMD.png.h
gdk-pixbuf-csource --raw --name=Centaur			data/pictures/Centaur.png	> embed/Centaur.png.h
gdk-pixbuf-csource --raw --name=Cyrix			data/pictures/Cyrix.png		> embed/Cyrix.png.h
gdk-pixbuf-csource --raw --name=Intel			data/pictures/Intel.png		> embed/Intel.png.h
gdk-pixbuf-csource --raw --name=National_Semiconductor  data/pictures/NSC.png		> embed/NSC.png.h
gdk-pixbuf-csource --raw --name=NexGen			data/pictures/NexGen.png	> embed/NexGen.png.h
gdk-pixbuf-csource --raw --name=Rise			data/pictures/Rise.png		> embed/Rise.png.h
gdk-pixbuf-csource --raw --name=SiS			data/pictures/SiS.png		> embed/SiS.png.h
gdk-pixbuf-csource --raw --name=Transmeta		data/pictures/Transmeta.png	> embed/Transmeta.png.h
gdk-pixbuf-csource --raw --name=UMC			data/pictures/UMC.png		> embed/UMC.png.h

echo -e "#ifndef CPUX_GLADE_H\n#define CPUX_GLADE_H\n\nstatic const char *cpux_glade =" > embed/cpu-x.ui.h
sed -e 's/\\/\\\\/g;s/"/\\"/g;s/ / /g;s/^/"/;s/$/\\n"/' 'data/cpux-gtk-3.8.ui' >> embed/cpu-x.ui.h
echo -e ";\n\n#endif\n" >> embed/cpu-x.ui.h
