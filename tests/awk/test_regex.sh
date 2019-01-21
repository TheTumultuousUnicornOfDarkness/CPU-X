#!/bin/bash
# shellcheck disable=SC2016

dir="$(mktemp -td CPU-X.XXXXXX)"

cd "$(dirname "$(realpath "$0")")" || exit 255
for awk in gawk mawk nawk; do
	$awk '/Sensor/ { print $5 }' aticonfig-odgt                         &> "$dir/aticonfig-odgt.$awk"
	$awk '/GPU Load/ { print $3 }' amdgpu_pm_info                       &> "$dir/amdgpu_pm_info.$awk"
	$awk -F '(: |Mhz)' '/\*/ { print $2 }' pp_dpm_sclk                  &> "$dir/pp_dpm_sclk.$awk"
	$awk -F '(: |Mhz)' '/\*/ { print $2 }' pp_dpm_mclk                  &> "$dir/pp_dpm_mclk.$awk"
	$awk '/GPU load/ { sub("%","",$4); print $4 }' aticonfig-odgc       &> "$dir/aticonfig-odgc-load.$awk"
	$awk '/Current Clocks/ { print $4 }' aticonfig-odgc                 &> "$dir/aticonfig-odgc-sclk.$awk"
	$awk '/Current Clocks/ { print $5 }' aticonfig-odgc                 &> "$dir/aticonfig-odgc-mclk.$awk"
	$awk -F '(sclk: | mclk:)' 'NR==2 { print $2 }' radeon_pm_info       &> "$dir/radeon_pm_info-sclk.$awk"
	$awk -F '(mclk: | vddc:)' 'NR==2 { print $2 }' radeon_pm_info       &> "$dir/radeon_pm_info-mclk.$awk"
	$awk -F '[,= ]' '{ print $2 }' nvidia-settings-GPUUtilization       &> "$dir/nvidia-settings-GPUUtilization.$awk"
	$awk -F '[,]'   '{ print $1 }' nvidia-settings-GPUCurrentClockFreqs &> "$dir/nvidia-settings-GPUCurrentClockFreqs-sclk.$awk"
	$awk -F '[,]'   '{ print $2 }' nvidia-settings-GPUCurrentClockFreqs &> "$dir/nvidia-settings-GPUCurrentClockFreqs-mclk.$awk"
done

cd "$dir" || exit 255
for file1 in *.gawk; do
	file2=$(basename "$file1" .gawk)
	printf "%-41s: " "$file2"
	if diff -qu "$file1" "$file2.mawk" && diff -qu "$file1" "$file2.nawk"; then
		echo -e "\033[1;32mOK\033[0m"
	fi
done

rm -rf "$dir"
