#!/bin/sh
# Combine the generic Cortex-A9 disk image with the PYNQ-Z1 BOOT.bin.
# Run inside the EDF container after an oe-init-build-env (wic must be on PATH).
#
#   assemble-sd.sh [output.wic]
set -eu

build=${EDF_BUILD:-"$HOME/build"}
common_wic="$build/cortexa9-common/tmp/deploy/images/amd-cortexa9thf-neon-common/edf-linux-disk-image-amd-cortexa9thf-neon-common.rootfs.wic"
boot_bin="$build/build/tmp/deploy/images/zynq-pynq-z1-sdt/BOOT-zynq-pynq-z1-sdt.bin"
out=${1:-"$build/pynq-z1-sd.wic"}

cp --reflink=auto -L "$common_wic" "$out"
wic cp "$boot_bin" "$out:1/boot.bin"
wic ls "$out:1"
sha256sum "$out"
