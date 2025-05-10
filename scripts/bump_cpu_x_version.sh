#!/usr/bin/env bash

# https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/wiki/release-a-version

set -euo pipefail

CPUX_DIR="$(git rev-parse --show-toplevel)"
TMP_CHANGELOG="$(mktemp --tmpdir cpu-x-XXXXXX)"
TMP_APPDATA="$(mktemp --tmpdir cpu-x-XXXXXX)"
DATE="$(date --iso-8601)"
GIT_CURRENT_BRANCH="$(git rev-parse --abbrev-ref HEAD)"
GIT_DEFAULT_BRANCH="$(git symbolic-ref refs/remotes/origin/HEAD | sed 's@^refs/remotes/origin/@@')"
PREVIOUS_VERSION="$(git describe --tags --abbrev=0)"
NEW_VERSION="${1:-}"


if [[ -z "$NEW_VERSION" ]]; then
	echo -n "Enter version: "
	read -r NEW_VERSION
fi

# Prepare ChangeLog
begin_changelog=false
while IFS= read -r line; do
	if ! $begin_changelog && [[ "$line" == "---" ]]; then
		begin_changelog=true
cat >> "$TMP_CHANGELOG" <<EOF
---

## [v$NEW_VERSION] - $DATE

### Added

-

### Changed

-

### Deprecated

-

### Removed

-

### Fixed

-

### Security

-

---
EOF
	else
		echo "$line" >> "$TMP_CHANGELOG"
	fi
done < "$CPUX_DIR/ChangeLog.md"
cp -f "$TMP_CHANGELOG" "$CPUX_DIR/ChangeLog.md"

# Update version in AppData
while IFS= read -r line; do
	if [[ "$line" =~ "<releases>" ]]; then
cat >> "$TMP_APPDATA" <<EOF
	<releases>
		<release version="${NEW_VERSION}" date="${DATE}">
			<url>https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/blob/${GIT_DEFAULT_BRANCH}/ChangeLog.md#v${NEW_VERSION//./}---${DATE}</url>
		</release>
EOF
	else
		echo "$line" >> "$TMP_APPDATA"
	fi
done < "$CPUX_DIR/data/io.github.thetumultuousunicornofdarkness.cpu-x.appdata.xml"
cp -f "$TMP_APPDATA" "$CPUX_DIR/data/io.github.thetumultuousunicornofdarkness.cpu-x.appdata.xml"

# Update version in CMakeLists.txt
sed -i "s|${PREVIOUS_VERSION/v/}|${NEW_VERSION}|" "$CPUX_DIR/CMakeLists.txt"

# Displaying changes since last tag
if [[ "$GIT_CURRENT_BRANCH" != "$GIT_DEFAULT_BRANCH" ]]; then
	git switch "$GIT_DEFAULT_BRANCH"
fi
git log --graph --decorate --oneline --color --first-parent "$GIT_DEFAULT_BRANCH" "$PREVIOUS_VERSION" | grep -Ev "Regen POT file|Update translation files|Translated using Weblate|Added translation using Weblate" | less --RAW-CONTROL-CHARS || true

# Wait for confirmation
while true; do
	read -r -p "Commit and push? " choice
	case "$choice" in
		y|Y|yes|YES) break;;
		n|N|no|NO) exit;;
		*) echo "Please answer yes or no.";;
	esac
done

# Commit, tag and push
git commit "$CPUX_DIR/ChangeLog.md" "$CPUX_DIR/CMakeLists.txt" "$CPUX_DIR/data/io.github.thetumultuousunicornofdarkness.cpu-x.appdata.xml" -m "Release version $NEW_VERSION"
git tag "v$NEW_VERSION"
git push --atomic origin "$GIT_DEFAULT_BRANCH" "v$NEW_VERSION"
