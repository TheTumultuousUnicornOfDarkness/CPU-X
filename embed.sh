#!/usr/bin/sh

# Put temporary stuff in "embed" directory
mkdir -p embed

# Convert PNG pictures to GDK Pixbuf
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

# Convert UI file to header file
file1="embed/cpu-x-3.8.ui.h"
file2="embed/cpu-x-3.14.ui.h"

## GTK 3.8+
cat << EOF > "$file1"
#ifndef CPUX_38_UI_H
#define CPUX_38_UI_H

static const char *cpux_38 =
EOF

sed -e 's/\\/\\\\/g;s/"/\\"/g;s/ / /g;s/^/"/;s/$/\\n"/' 'data/cpux-gtk-3.8.ui' >> "$file1"

cat << EOF >> "$file1"
;

#endif

EOF

## GTK 3.14+
cat << EOF > "$file2"
#ifndef CPUX_314_UI_H
#define CPUX_314_UI_H

static const char *cpux_314 =
EOF

sed -e 's/\\/\\\\/g;s/"/\\"/g;s/ / /g;s/^/"/;s/$/\\n"/' 'data/cpux-gtk-3.14.ui' >> "$file2"

cat << EOF >> "$file2"
;

#endif

EOF
