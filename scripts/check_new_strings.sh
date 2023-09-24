#!/bin/bash
# This script is used to check if there are new strings in source files

set -euo pipefail

GIT_DIR="$(git rev-parse --show-toplevel)"
SHA=${GITHUB_SHA:-HEAD~0}
cd "$GIT_DIR" || exit 1

if [[ "$(git show -s --format='%ce' "$SHA")" == "hosted@weblate.org" ]]; then
	echo "Commit by Weblate, nothing to do." > /dev/stderr
	echo "NO_CHANGE"
	exit 0
fi

for file in $(git show --pretty="" --name-only "$SHA"); do
	if [[ "$file" =~ \.(c|h|cpp|hpp|ui)$ ]]; then
		if git show "$SHA" "$file" | grep -E '^(\+|\-)' | grep -E -qw '_\(|N_\('; then
			git show "$SHA" "$file" > /dev/stderr
			echo "REGEN"
			exit 0
		fi
	fi
done

echo "No commit with translation change." > /dev/stderr
echo "NO_CHANGE"
