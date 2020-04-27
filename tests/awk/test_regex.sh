#!/bin/bash

TEST_DIR="$(dirname "$(realpath "$0")")"

# shellcheck disable=SC1090
source "$TEST_DIR/../common.sh"
cd "$TEST_DIR" || exit 255

tmp_dir="$(mktemp -td CPU-X.XXXXXX)"
for awk in gawk mawk nawk; do
	$awk '/Sensor/ { print $5 }' aticonfig-odgt                         &> "$tmp_dir/aticonfig-odgt.$awk"
	$awk '/GPU Load/ { print $3 }' amdgpu_pm_info                       &> "$tmp_dir/amdgpu_pm_info.$awk"
	$awk -F '(: |Mhz)' '/\*/ { print $2 }' pp_dpm_sclk                  &> "$tmp_dir/pp_dpm_sclk.$awk"
	$awk -F '(: |Mhz)' '/\*/ { print $2 }' pp_dpm_mclk                  &> "$tmp_dir/pp_dpm_mclk.$awk"
	$awk '/GPU load/ { sub("%","",$4); print $4 }' aticonfig-odgc       &> "$tmp_dir/aticonfig-odgc-load.$awk"
	$awk '/Current Clocks/ { print $4 }' aticonfig-odgc                 &> "$tmp_dir/aticonfig-odgc-sclk.$awk"
	$awk '/Current Clocks/ { print $5 }' aticonfig-odgc                 &> "$tmp_dir/aticonfig-odgc-mclk.$awk"
	$awk -F '(sclk: | mclk:)' 'NR==2 { print $2 }' radeon_pm_info       &> "$tmp_dir/radeon_pm_info-sclk.$awk"
	$awk -F '(mclk: | vddc:)' 'NR==2 { print $2 }' radeon_pm_info       &> "$tmp_dir/radeon_pm_info-mclk.$awk"
	$awk -F '[,= ]' '{ print $2 }' nvidia-settings-GPUUtilization       &> "$tmp_dir/nvidia-settings-GPUUtilization.$awk"
	$awk -F '[,]'   '{ print $1 }' nvidia-settings-GPUCurrentClockFreqs &> "$tmp_dir/nvidia-settings-GPUCurrentClockFreqs-sclk.$awk"
	$awk -F '[,]'   '{ print $2 }' nvidia-settings-GPUCurrentClockFreqs &> "$tmp_dir/nvidia-settings-GPUCurrentClockFreqs-mclk.$awk"
done

cd "$tmp_dir" || exit 255
failed=0
for file1 in *.gawk; do
	file2=$(basename "$file1" .gawk)
	printf "%-41s: " "$file2"
	if diff -qu "$file1" "$file2.mawk" && diff -qu "$file1" "$file2.nawk"; then
		success "OK"
	else
		error "KO" "(got '$(< "$file2.mawk")'/'$(< "$file2.nawk")', wanted $(< "$file1")"
		((failed++))
	fi
done
summary "$failed"
rm -rf "$tmp_dir"
