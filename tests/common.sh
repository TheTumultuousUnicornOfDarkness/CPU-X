#!/usr/bin/env bash

success() {
	local msg="$1"
	local details="$2"
	echo -e "\033[1;32m$msg\033[0m $details"
}

error() {
	local msg="$1"
	local details="$2"
	echo -e "\033[1;31m$msg\033[0m $details"
}

summary() {
	local failed=$1
	echo
	if [[ $failed -eq 0 ]]; then
		success "All tests passed!"
	else
		error "$failed tests failed."
	fi
	exit "$failed"
}
