#!/bin/bash
# This script is used to check if there are new strings in source files

set -euo pipefail

GIT_DIR="$(git rev-parse --show-toplevel)"
cd "$GIT_DIR" || exit 1

if [[ "$(git show -s --format='%ce')" == "hosted@weblate.org" ]]; then
	echo "Last commit by WebLate, skipping commit."
	exit 1
fi

for file in $(git show --pretty="" --name-only); do
	if [[ "$file" == *.c ]] || [[ "$file" == *.h ]]; then
		if git show "$file" | egrep '^(\+|\-)' | egrep -qw '_\(|N_\('; then
			echo "Found a string change in $file."
			exit 0
		fi
	fi
done

echo "No string change found."
exit 1
