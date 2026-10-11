#!/bin/sh
set -eu

BUILDROOT_DIR="${BUILDROOT_DIR:-"$HOME/buildroot"}"
exec "$BUILDROOT_DIR/output/images/start-qemu.sh" "$@"
