# AMD EDF image for PYNQ-Z2

Builds an SD card image for the PYNQ-Z2 (Zynq-7000, dual Cortex-A9) with
`sysmon` installed as a systemd service, using AMD EDF 26.06 (`rel-v2026.1`,
Yocto scarthgap). Same approach as the PYNQ-Z1 build in `README.md`.

```
yocto/
  edf-manifest.xml          upstream layer revisions this was built against
  meta-pynq-z2/             board layer (machine conf, FSBL multiconfig, device trees)
    sdt/zynq-pynq-z2/       system device tree sources
  meta-sysmon/              sysmon-web recipe (builds this repo) + systemd unit
  scripts/
    extract-xsa-z2.sh       copies bitstream + ps7_init from the XSA into the SDT dir
    assemble-sd-z2.sh       combines the rootfs .wic with the board BOOT.bin
hardware/sysmon-z2-fpga.xsa Vivado hardware export
```

Two build directories, as with the Z1: `build-pynq` (`MACHINE = zynq-pynq-z2`)
produces `BOOT.bin`; `build-cortexa9` (`amd-cortexa9thf-neon-common`) produces
the kernel + rootfs and reuses AMD's prebuilt sstate. Rootfs changes go in
`meta-sysmon`.

## Build

### 0. Container and sources

```sh
# on the host
docker run -it --name edf-z2 xilinx/edf:ubuntu2204-26.06 bash
docker cp sysmon-proj edf-z2:/home/amd-edf/sysmon-proj
```

Inside the container (as `amd-edf`), sync the pinned layers. `repo` only reads
manifests from a git repo, so wrap the pinned one in a local repo:

```sh
mkdir -p ~/edf-manifest && cp ~/sysmon-proj/yocto/edf-manifest.xml ~/edf-manifest/default.xml
git -C ~/edf-manifest init -q && git -C ~/edf-manifest add . && git -C ~/edf-manifest commit -qm pin
mkdir ~/pynq-z2-edf && cd ~/pynq-z2-edf
repo init -u file://$HOME/edf-manifest && repo sync

~/sysmon-proj/yocto/scripts/extract-xsa-z2.sh
```

Add the layers from their place in the repo, not as copies; the sysmon recipe
finds the source relative to its own location.

### 1. Board build (`BOOT.bin`)

```sh
cd ~/pynq-z2-edf
source edf-init-build-env build-pynq
bitbake-layers add-layer ~/sysmon-proj/yocto/meta-pynq-z2 ~/sysmon-proj/yocto/meta-sysmon
```

Append to `conf/local.conf`:

```
MACHINE = "zynq-pynq-z2"
SKIP_META_VIRT_SANITY_CHECK = "1"
SKIP_META_SECURITY_SANITY_CHECK = "1"
SKIP_META_TPM_SANITY_CHECK = "1"
```

```sh
bitbake xilinx-bootbin
```

### 2. Common rootfs build

```sh
cd ~/pynq-z2-edf
source edf-init-build-env build-cortexa9
bitbake-layers add-layer ~/sysmon-proj/yocto/meta-sysmon
```

Append to `conf/local.conf` (thread counts are optional, sized for a 4-core VM):

```
MACHINE = "amd-cortexa9thf-neon-common"
BB_NUMBER_THREADS = "4"
PARALLEL_MAKE = "-j 4"
```

```sh
bitbake edf-linux-disk-image
```

### 3. Assemble the SD image

```sh
~/sysmon-proj/yocto/scripts/assemble-sd-z2.sh     # -> ~/pynq-z2-edf/pynq-z2-sd.wic
```

```sh
# on the host
docker cp edf-z2:/home/amd-edf/pynq-z2-edf/pynq-z2-sd.wic .
```

## Regenerating the machine conf

Only needed if the XSA changes. Generate the SDT from the XSA with sdtgen
(Vitis), then:

```sh
export PATH="$PATH:$HOME/pynq-z2-edf/sources/meta-xilinx/meta-xilinx-core/gen-machine-conf"
gen-machine-conf parse-sdt \
  --hw-description <sdt-dir> \
  -c ~/sysmon-proj/yocto/meta-pynq-z2/conf \
  -l ~/pynq-z2-edf/build-pynq/conf/local.conf \
  --machine-name zynq-pynq-z2
```

Afterwards, put the relative `SDT_URI` lines back in
`conf/machine/zynq-pynq-z2.conf` and copy the new device-tree sources (not the
`.bit` or `ps7_init*` files) into `sdt/zynq-pynq-z2/`.
