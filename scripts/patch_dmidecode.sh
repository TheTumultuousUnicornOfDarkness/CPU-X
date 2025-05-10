#!/usr/bin/env bash

set -euo pipefail

CPUX_DIR="$(git rev-parse --show-toplevel)/src/core/dmidecode"
DMIDECODE_DIR="/tmp/dmidecode"
HASH_FILE="$CPUX_DIR/.hash"
OLD_HASH=$(cat "$HASH_FILE")
OLD_HASH_SHORT=${OLD_HASH:0:7}

# Get dmidecode source code
git clone https://git.savannah.gnu.org/git/dmidecode.git "$DMIDECODE_DIR"
cd "$DMIDECODE_DIR" || exit 255

# Retrieve informations about repo
VER=$(git describe --abbrev=0 --tags | sed 's/^dmidecode-//;s/-/./')
DATE=$(git show -s --format=%cd --date=format:%Y%m%d)
NEW_HASH=$(git rev-parse HEAD)
NEW_HASH_SHORT=$(git rev-parse --short HEAD)
git diff "$OLD_HASH" master > "dmidecode.patch"

# Patch to the new version
cd "$CPUX_DIR" || exit 255
echo "$NEW_HASH" > "$HASH_FILE"
sed -i "s/$OLD_HASH/$NEW_HASH/"                      "README.md"
sed -i "s/$OLD_HASH_SHORT/$NEW_HASH_SHORT/"          "README.md"
sed -i "s/VERSION \"[^ ]*\"/VERSION \"$VER.$DATE\"/" "CMakeLists.txt"
patch --batch --no-backup-if-mismatch --input="$DMIDECODE_DIR/dmidecode.patch"

# Commit
git commit "$CPUX_DIR" -m "Patch dmidecode to version $VER commit $NEW_HASH_SHORT"
