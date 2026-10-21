# AMD EDF image for PYNQ-Z1

Builds an SD card image for the PYNQ-Z1 (Zynq-7000, dual Cortex-A9,
armv7 hard-float) with AMD EDF 26.06 (Yocto scarthgap).

```
yocto/
  meta-pynq-z1/              board layer (machine conf, FSBL multiconfig, device trees)
    sdt/zynq-pynq-z1-sdt/    system device tree sources
  scripts/
    extract-xsa.sh           copies bitstream + ps7_init from the XSA into the SDT dir
    assemble-sd.sh           combines the rootfs .wic with the board BOOT.bin
hardware/pynq-z1-ps-only.xsa Vivado hardware export (PS-only design)
```

## Why two build directories

EDF splits a Zynq build in two:

| Build dir                 | `MACHINE`                     | Produces |
|---------------------------|-------------------------------|----------|
| `~/build/build`           | `zynq-pynq-z1-sdt`            | `BOOT.bin` (FSBL, bitstream, U-Boot, device tree) |
| `~/build/cortexa9-common` | `amd-cortexa9thf-neon-common` | `edf-linux-disk-image` (kernel + rootfs) |

The rootfs is built for a generic Cortex-A9 machine, which lets it reuse AMD's
prebuilt sstate. Only the boot firmware is specific to the board. The final SD
image is the common `.wic` with the board's `BOOT.bin` copied into its
boot partition. Rootfs changes (adding packages, services) go in the
`cortexa9-common` build.

## Build

All commands run inside the EDF container as `amd-edf`.

```sh
docker start edf-dev
docker exec -it -u amd-edf edf-dev bash
```

### 0. Put the repo in the container and add the bitstream

```sh
# on the host
docker cp sysmon-proj edf-dev:/home/amd-edf/sysmon-proj
# in the container
~/sysmon-proj/yocto/scripts/extract-xsa.sh
```

### 1. Board build (`BOOT.bin`)

```sh
source ~/sources/poky/oe-init-build-env ~/build/build
bitbake-layers add-layer ~/sysmon-proj/yocto/meta-pynq-z1
```

Append to `conf/local.conf`. gen-machine-conf wrote these lines; the layer
already contains the machine conf it generated, so it doesn't need to run again:

```
MACHINE = "zynq-pynq-z1-sdt"
SKIP_META_VIRT_SANITY_CHECK = "1"
SKIP_META_SECURITY_SANITY_CHECK = "1"
SKIP_META_TPM_SANITY_CHECK = "1"
```

```sh
bitbake xilinx-bootbin
```

### 2. Common rootfs build

```sh
mkdir -p ~/build/cortexa9-common/conf
cp ~/build/build/conf/bblayers.conf ~/build/build/conf/local.conf ~/build/cortexa9-common/conf/
sed -i 's/^MACHINE = "zynq-pynq-z1-sdt"/MACHINE = "amd-cortexa9thf-neon-common"/' \
    ~/build/cortexa9-common/conf/local.conf

source ~/sources/poky/oe-init-build-env ~/build/cortexa9-common
bitbake edf-linux-disk-image
```

### 3. Assemble the SD image

```sh
~/sysmon-proj/yocto/scripts/assemble-sd.sh            # -> ~/build/pynq-z1-sd.wic
```

Copy it to the host and write it to the SD card:

```sh
docker cp edf-dev:/home/amd-edf/build/pynq-z1-sd.wic .
```

## Regenerating the machine conf

Only needed if the XSA changes. Generate the SDT from the XSA with sdtgen
(Vitis), then:

```sh
export PATH="$PATH:$HOME/sources/meta-xilinx/meta-xilinx-core/gen-machine-conf"
gen-machine-conf parse-sdt \
  --hw-description <sdt-dir> \
  -c ~/sysmon-proj/yocto/meta-pynq-z1/conf \
  -l ~/build/build/conf/local.conf \
  --machine-name zynq-pynq-z1-sdt
```

Afterwards, put the relative `SDT_URI` lines back in
`conf/machine/zynq-pynq-z1-sdt.conf` and copy the new device-tree sources into
`sdt/zynq-pynq-z1-sdt/`.
