#!/bin/bash

TEST_DIR="$(dirname "$(realpath "$0")")"

# shellcheck disable=SC1090
source "$TEST_DIR/../common.sh"
cd "$TEST_DIR" || exit 255

failed=0
for samplefile in samples/nouveau_pstate_*; do
	file=$(basename "$samplefile")
	pstate=$(grep '\*' "$samplefile" || sed -n 1p "$samplefile")
	samplecore=$(echo "$pstate" | grep -oP '(?<=core )[^ ]*' | cut -d- -f2)
	samplememory=$(echo "$pstate" | grep -oP '(?<=memory )[^ ]*')
	resultcore=$(< "results/${file}_core")
	resultmemory=$(< "results/${file}_memory")
	printf "%-41s: " "$file"
	if [[ "$samplecore" == "$resultcore" ]] && [[ "$samplememory" == "$resultmemory" ]]; then
		success "OK"
	else
		error "KO" "(got $samplecore/$samplememory, wanted $resultcore/$resultmemory)"
		((failed++))
	fi
done
summary "$failed"
