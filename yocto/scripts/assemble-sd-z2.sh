#!/bin/sh
# Combine the generic Cortex-A9 disk image with the PYNQ-Z2 BOOT.bin.
# Run inside the EDF container after sourcing edf-init-build-env (wic must be
# on PATH).
#
#   assemble-sd-z2.sh [output.wic]
set -eu

ws=${EDF_WS:-"$HOME/pynq-z2-edf"}
common_wic="$ws/build-cortexa9/tmp/deploy/images/amd-cortexa9thf-neon-common/edf-linux-disk-image-amd-cortexa9thf-neon-common.rootfs.wic"
boot_bin="$ws/build-pynq/tmp/deploy/images/zynq-pynq-z2/BOOT-zynq-pynq-z2.bin"
out=${1:-"$ws/pynq-z2-sd.wic"}

cp --reflink=auto -L "$common_wic" "$out"
wic cp "$boot_bin" "$out:1/boot.bin"
wic ls "$out:1"
sha256sum "$out"
