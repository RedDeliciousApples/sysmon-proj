#!/bin/sh
# Copy the generated hardware files (bitstream, ps7_init) out of the XSA into
# the PYNQ-Z2 layer's SDT directory. They are byte-identical to what sdtgen
# produced, so only the device-tree sources are kept in git.
set -eu

here=$(cd "$(dirname "$0")" && pwd)
xsa=${1:-"$here/../../hardware/sysmon-z2-fpga.xsa"}
sdt="$here/../meta-pynq-z2/sdt/zynq-pynq-z2"

unzip -o -j "$xsa" \
    sysmon-z2-fpga.bit \
    ps7_init.c ps7_init.h ps7_init.html ps7_init.tcl \
    ps7_init_gpl.c ps7_init_gpl.h \
    -d "$sdt"
