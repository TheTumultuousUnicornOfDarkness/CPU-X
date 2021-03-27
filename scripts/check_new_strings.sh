#!/bin/bash
# This script is used to create archives for releases

set -euo pipefail

GIT_DIR="$(git rev-parse --show-toplevel)"

if [[ "$(git show -s --format='%ce' HEAD~0)" == "hosted@weblate.org" ]]; then
	echo "Last commit by WebLate, skipping commit."
	exit 1
fi

for file in $(git diff --name-only HEAD~0 HEAD~1); do
	if [[ "$file" == *.c ]] || [[ "$file" == *.h ]]; then
		if git show "$file" | egrep '^(\+|\-)' | egrep -qw '_\(|N_\('; then
			echo "Found a string change in $file."
			exit 0
		fi
	fi
done

echo "No string change found."
exit 1
