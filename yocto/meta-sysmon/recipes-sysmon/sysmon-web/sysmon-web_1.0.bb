SUMMARY = "Lightweight HTTP system monitor"
DESCRIPTION = "Collects Linux system metrics and exposes them through an HTTP endpoint."
LICENSE = "CLOSED"

# Build straight from this repository (four levels above this recipe) instead
# of a copy inside the layer. Objects go to ${B} so the host's ./build is left
# alone. Add the layer from its place in the repo (bitbake-layers add-layer
# <repo>/yocto/meta-sysmon); copying the layer elsewhere breaks this path.
inherit externalsrc
EXTERNALSRC = "${@os.path.abspath(os.path.join(d.getVar('FILE_DIRNAME'), '../../../..'))}"
EXTERNALSRC_BUILD = "${WORKDIR}/build"

SRC_URI = "file://sysmon.service"

inherit systemd

SYSTEMD_SERVICE:${PN} = "sysmon.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_compile() {
    oe_runmake -C ${S} \
        'BUILD_DIR=${B}' \
        'CXX=${CXX}' \
        'CPPFLAGS=${CPPFLAGS} -Iinclude -Iexternal -MMD -MP' \
        'CXXFLAGS=${CXXFLAGS} -std=c++17 -Wall -Wextra -Wpedantic -Wshadow -pthread' \
        'LDFLAGS=${LDFLAGS} -pthread'
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/sysmon ${D}${bindir}/sysmon

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/sysmon.service \
        ${D}${systemd_system_unitdir}/sysmon.service
}

FILES:${PN} += "${systemd_system_unitdir}/sysmon.service"
