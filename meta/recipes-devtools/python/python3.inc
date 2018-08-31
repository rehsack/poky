SUMMARY = "The Python Programming Language"
HOMEPAGE = "http://www.python.org"
LICENSE = "PSFv2"
SECTION = "devel/python"

PYTHON_MAJMIN = "3.7"
DISTRO_SRC_URI ?= "file://sitecustomize.py"
DISTRO_SRC_URI_linuxstdbase = ""
SRC_URI = "http://www.python.org/ftp/python/${PV}/Python-${PV}.tar.xz \
    file://python-config.patch \
    file://python-3.3-multilib.patch \
    file://03-fix-tkinter-detection.patch \
    file://avoid_warning_about_tkinter.patch \
    file://unixccompiler.patch \
    file://sysroot-include-headers.patch \
    file://sysconfig.py-add-_PYTHON_PROJECT_SRC.patch \
    file://setup.py-check-cross_compiling-when-get-FLAGS.patch \
    file://030-fixup-include-dirs.patch \
    file://070-dont-clean-ipkg-install.patch \
    file://080-distutils-dont_adjust_files.patch \
    file://130-readline-setup.patch \
    file://0001-h2py-Fix-issue-13032-where-it-fails-with-UnicodeDeco.patch \
    ${DISTRO_SRC_URI} \
    file://support_SOURCE_DATE_EPOCH_in_py_compile.patch \
    file://Use-correct-CFLAGS-for-extensions-when-cross-compili.patch \
"

SRC_URI[md5sum] = "eb8c2a6b1447d50813c02714af4681f3"
SRC_URI[sha256sum] = "0382996d1ee6aafe59763426cf0139ffebe36984474d0ec4126dd1c40a8b3549"

LIC_FILES_CHKSUM = "file://LICENSE;md5=f257cc14f81685691652a3d3e1b5d754"

# exclude pre-releases for both python 2.x and 3.x
UPSTREAM_CHECK_REGEX = "[Pp]ython-(?P<pver>\d+(\.\d+)+).tar"

S = "${WORKDIR}/Python-${PV}"

CVE_PRODUCT = "python"

inherit autotools bluetooth pkgconfig python3-dir

EXTRA_OECONF = "\
  --with-pymalloc \
  --without-cxx-main \
  --enable-shared \
  --enable-ipv6=${@bb.utils.contains('DISTRO_FEATURES', 'ipv6', 'yes', 'no', d)} \
"

PACKAGECONFIG[bluetooth] = ",ac_cv_header_bluetooth_bluetooth_h=no ac_cv_header_bluetooth_h=no,${BLUEZ}"

do_configure_prepend() {
	libdirleaf="$(echo ${libdir} | sed -e 's:${prefix}/::')"
	sed -i -e "s:SEDMELIBLEAF:${libdirleaf}:g" \
		${S}/configure.ac
}

do_install_prepend() {
	MAKESETTINGS="$(egrep '^(ABIFLAGS|MULTIARCH)=' ${B}/Makefile | sed -E -e 's/[[:space:]]//g' -e 's/=/="/' -e 's/$/"/')"
	eval ${MAKESETTINGS}
	if test "${ABIFLAGS}" != "${PYTHON_ABI}"; then
	    die "do_install: configure determined ABIFLAGS '${ABIFLAGS}' != '${PYTHON_ABI}' from python3-dir.bbclass"
	fi
	if test "x${BUILD_OS}" = "x${TARGET_OS}"; then
		# no cross-compile at all
		_PYTHON_SYSCONFIGDATA_NAME=${PYTHON_ABI}_${TARGET_OS}_${MULTIARCH}
	else
		# at the very moment, it's the only available target
		_PYTHON_SYSCONFIGDATA_NAME=${PYTHON_ABI}_linux_${MULTIARCH}
	fi
}

do_install_append () {
	sed -i -e 's:${HOSTTOOLS_DIR}/install:install:g' \
		-e 's:${HOSTTOOLS_DIR}/mkdir:mkdir:g' \
		${D}/${libdir}/python${PYTHON_MAJMIN}/_sysconfigdata_${_PYTHON_SYSCONFIGDATA_NAME}.py
}
