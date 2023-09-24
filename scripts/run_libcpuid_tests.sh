#!/bin/bash
# This script is used to test the CPU Technology database

set -euo pipefail

CR="\033[0m"     # Color reset
CB="\033[1m"     # Color bold
CBR="\033[1;31m" # Color bold red
CBG="\033[1;32m" # Color bold green

if [[ $# -lt 1 ]]; then
	echo "$0: you must provide the libcpuid 'tests' directory."
	exit 1
else
	TESTS_DIR="$1"
fi

count=0
while IFS= read -r -d '' file; do
	if [[ ! "$file" =~ "amd" ]] && [[ ! "$file" =~ "intel" ]]; then
		# Only AMD and Intel CPUs are supported
		continue
	fi

	# Cut test file after delimiter (dash line)
	tmp_file="/tmp/$(basename "$file")"
	while read -r line; do
		if [[ "$line" == "--------------------------------------------------------------------------------" ]]; then
			break
		fi
		echo "$line" >> "$tmp_file"
	done < "$file"

	# Run with CPU-X and display test status
	printf "Test file ${CB}%-32s${CR}: " "$(basename "$file")"
	if output=$(LC_ALL=C CPUX_CPUID_RAW="$tmp_file" CPUX_DEBUG_DATABASE=1 cpu-x --dump --nocolor 2> /dev/null); then
		echo -en "${CBG}OK${CR}\n"
	else
		echo -en "${CBR}${output#*"==> "}${CR}\n"
		((count++))
	fi
	rm "$tmp_file"
done < <(find "$TESTS_DIR" -name '*.test' -print0)

echo -en "\nDone: "
if [[ $count -eq 0 ]]; then
	echo -e "${CBG}all test files passed with success!${CR}"
else
	echo -e "${CBR}$count test files failed to pass.${CR}"
fi
exit "$count"
