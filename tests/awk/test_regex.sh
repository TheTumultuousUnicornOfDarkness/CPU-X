#!/bin/bash

TEST_DIR="$(dirname "$(realpath "$0")")"

# shellcheck disable=SC1090
source "$TEST_DIR/../common.sh"
cd "$TEST_DIR/samples" || exit 255

tmp_dir="$(mktemp --tmpdir --directory CPU-X.XXXXXX)"
for awk in gawk mawk nawk; do
	if [[ -z "$(command -v "$awk")" ]]; then
		continue
	fi
	$awk '/Sensor/ { print $5 }' aticonfig-odgt                         &> "$tmp_dir/aticonfig-odgt.$awk"
	$awk '/GPU Load/ { print $3 }' amdgpu_pm_info                       &> "$tmp_dir/amdgpu_pm_info.$awk"
	$awk -F '(: |Mhz)' '/\*/ { print $2 }' pp_dpm_sclk                  &> "$tmp_dir/pp_dpm_sclk.$awk"
	$awk -F '(: |Mhz)' '/\*/ { print $2 }' pp_dpm_mclk                  &> "$tmp_dir/pp_dpm_mclk.$awk"
	$awk '/GPU load/ { sub("%","",$4); print $4 }' aticonfig-odgc       &> "$tmp_dir/aticonfig-odgc-load.$awk"
	$awk '/Current Clocks/ { print $4 }' aticonfig-odgc                 &> "$tmp_dir/aticonfig-odgc-sclk.$awk"
	$awk '/Current Clocks/ { print $5 }' aticonfig-odgc                 &> "$tmp_dir/aticonfig-odgc-mclk.$awk"
	$awk -F '(sclk: | mclk:)'  'NR==2 { print $2 }' radeon_pm_info      &> "$tmp_dir/radeon_pm_info-sclk.$awk"
	$awk -F '(mclk: | vddc:)'  'NR==2 { print $2 }' radeon_pm_info      &> "$tmp_dir/radeon_pm_info-mclk.$awk"
	$awk -F '(vddc: | vddci:)' 'NR==2 { print $2 }' radeon_pm_info      &> "$tmp_dir/radeon_pm_info-vddc.$awk"
	$awk -F '[,= ]' '{ print $2 }' nvidia-settings-GPUUtilization       &> "$tmp_dir/nvidia-settings-GPUUtilization.$awk"
	$awk -F '[,]'   '{ print $1 }' nvidia-settings-GPUCurrentClockFreqs &> "$tmp_dir/nvidia-settings-GPUCurrentClockFreqs-sclk.$awk"
	$awk -F '[,]'   '{ print $2 }' nvidia-settings-GPUCurrentClockFreqs &> "$tmp_dir/nvidia-settings-GPUCurrentClockFreqs-mclk.$awk"
done

cd "$tmp_dir" || exit 255
failed=0
for res_file in *; do
	exp_file="$TEST_DIR/results/$(basename "${res_file%.*}")"
	printf "%-50s: " "$res_file"
	if cmp --quiet "$exp_file" "$res_file"; then
		success "OK"
	else
		error "KO" "(got '$(< "$res_file")', wanted '$(< "$exp_file")')"
		((failed++))
	fi
done
summary "$failed"
rm -rf "$tmp_dir"
