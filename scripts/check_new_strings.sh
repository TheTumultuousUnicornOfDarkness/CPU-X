#!/bin/bash -x
# This script is used to check if there are new strings in source files

set -euo pipefail

GIT_DIR="$(git rev-parse --show-toplevel)"
SHA=${GITHUB_SHA:-HEAD~0}
cd "$GIT_DIR" || exit 1

if [[ "$(git show -s --format='%ce' "$SHA")" == "hosted@weblate.org" ]]; then
	echo "NO_CHANGE"
	exit 0
fi

for file in $(git show --pretty="" --name-only "$SHA"); do
	if [[ "$file" == *.c ]] || [[ "$file" == *.h ]]; then
		if git show "$SHA" "$file" | egrep '^(\+|\-)' | egrep -qw '_\(|N_\('; then
			#echo "REGEN"
			echo "NO_CHANGE"
			exit 0
		fi
	fi
done

echo "NO_CHANGE"
