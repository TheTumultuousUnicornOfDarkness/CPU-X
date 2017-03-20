#!/bin/bash
# This script is used to test the CPU Technology database

TESTS_DIR=$1
CR="\033[0m"     # Color reset
CB="\033[1m"     # Color bold
CBR="\033[1;31m" # Color bold red
CBG="\033[1;32m" # Color bold green

if [[ $# -lt 1 ]] || [[ ! -d "$TESTS_DIR" ]]; then
	printf "$0: you must provide the libcpuid 'tests' directory."
	exit 1
fi

count=0
for file in $(find $TESTS_DIR -name *.test); do
	# Cut test file after delimiter (dash line)
	tmp_file="/tmp/$(basename $file)"
	while read -r line; do
		echo $line | grep -q "\-\-\-\-\-\-\-\-\-\-\-\-" && break
		echo $line >> "$tmp_file"
	done < $file

	# Run with CPU-X
	printf "Test file ${CB}%-32s${CR}: " "$(basename $file)"
	output=$(sudo LC_ALL=C CPUX_CPUID_RAW="$tmp_file" CPUX_DEBUG_DATABASE=1 cpu-x --dump --nocolor)

	# Display test status
	if [[ $? -gt 0 ]]; then
		printf "${CBR}${output#*"==> "}${CR}"
		((count++))
	else
		printf "${CBG}OK${CR}\n"
	fi
	rm "$tmp_file"
done

printf "\nDone: "
if [[ $count -eq 0 ]]; then
	printf "${CBG}all test files passed with success!${CR}\n"
else
	printf "${CBR}$count test files failed to pass.${CR}\n"
fi
