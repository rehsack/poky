require recipes-devtools/python/python3.inc

SRC_URI += "\
    file://12-distutils-prefix-is-inside-staging-area.patch \
    file://0001-Do-not-use-the-shell-version-of-python-config-that-w.patch \
"

EXTRANATIVEPATH += "bzip2-native"
DEPENDS = "openssl-native libffi-native bzip2-replacement-native zlib-native \
           util-linux-native readline-native sqlite3-native gdbm-native \
"

inherit native

EXTRA_OECONF_append = " --bindir=${bindir}/${PN} --without-ensurepip"

EXTRA_OEMAKE = '\
  LIBC="" \
  STAGING_LIBDIR=${STAGING_LIBDIR_NATIVE} \
  STAGING_INCDIR=${STAGING_INCDIR_NATIVE} \
  LIB=${baselib} \
  ARCH=${TARGET_ARCH} \
'

# Regenerate all of the generated files
# This ensures that pgen and friends get created during the compile phase
#
do_compile_prepend() {
    # Assuming https://bugs.python.org/issue33080 has been addressed in Makefile.
    oe_runmake regen-all
}

do_install() {
	install -d ${D}${libdir}/pkgconfig
	oe_runmake 'DESTDIR=${D}' install
	if [ -e ${WORKDIR}/sitecustomize.py ]; then
		install -m 0644 ${WORKDIR}/sitecustomize.py ${D}/${libdir}/python${PYTHON_MAJMIN}
	fi
	install -d ${D}${bindir}/${PN}
	install -m 0755 Parser/pgen ${D}${bindir}/${PN}

	# Make sure we use /usr/bin/env python
	for PYTHSCRIPT in `grep -rIl ${bindir}/${PN}/python ${D}${bindir}/${PN}`; do
		sed -i -e '1s|^#!.*|#!/usr/bin/env python3|' $PYTHSCRIPT
	done

        # Add a symlink to the native Python so that scripts can just invoke
        # "nativepython" and get the right one without needing absolute paths
        # (these often end up too long for the #! parser in the kernel as the
        # buffer is 128 bytes long).
        ln -s python3-native/python3 ${D}${bindir}/nativepython3
}

python(){

    # Read JSON manifest
    import json
    pythondir = d.getVar('THISDIR',True)
    with open(pythondir+'/python3/python3-manifest.json') as manifest_file:
        python_manifest=json.load(manifest_file)

    rprovides = d.getVar('RPROVIDES').split()

    # Hardcoded since it cant be python3-native-foo, should be python3-foo-native
    pn = 'python3'

    for key in python_manifest:
        pypackage = pn + '-' + key + '-native'
        if pypackage not in rprovides:
              rprovides.append(pypackage)

    d.setVar('RPROVIDES', ' '.join(rprovides))
}
