#!/bin/bash
# shellcheck disable=SC1090

#export USE_CCACHE=1

KERNEL_DEVMODEL="${KERNEL_DEVMODEL:-US997}"
export KERNEL_DEVMODEL

SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"


errchk cd "${KERNEL_SOURCE_DIR}"

Z_BUILD_SCRIPT="${SCRIPTDIR}/make.sh"
Z_BUILD_SCRIPT_PARAM=("dtbs" "Image.gz-dtb")

Z_CONFIG_OVERRIDE=()

while [ "${1}" != "" ]; do
                case "${1}" in
			"build")
				shift
				errchk "${SCRIPTDIR}/create_defconfigs.sh" "${Z_CONFIG_OVERRIDE[@]}"
				errchk "${SCRIPTDIR}/defconfig.sh"
				errchk "${Z_BUILD_SCRIPT}" "${Z_BUILD_SCRIPT_PARAM[@]}"
				;;

			"zip")
				shift
				errchk "${SCRIPTDIR}/buildzip.sh"
				;;

			"clean")
				shift
				errchk rm -rf "${KERNEL_SOURCE_DIR:?}/${KERNEL_BUILD_DIR:?}"
				errchk "${SCRIPTDIR}/clean.sh"
				;;

			"gitreset")
				shift
				errchk rm -rf "${KERNEL_SOURCE_DIR:?}/${KERNEL_BUILD_DIR:?}"
				export FORCE_RESET=1
				errchk "${SCRIPTDIR}/clean.sh"
				export FORCE_RESET=
				;;

			"prodtest")
				shift
				"${0}" gitreset
				Z_BUILD_SCRIPT="${SCRIPTDIR}/do_kernel_build.sh"
				Z_BUILD_SCRIPT_PARAM=("${KERNEL_DEVMODEL}")
				"${Z_BUILD_SCRIPT}" "${Z_BUILD_SCRIPT_PARAM[@]}"
				exit $?
				;;

			"debug")
				Z_CONFIG_OVERRIDE+=("CONFIG_MELINA_DEBUG_DISABLE=n" "CONFIG_MELINA_DEBUG_ENABLE=y")
				USE_DEBUG=1
				export Z_CONFIG_OVERRIDE USE_DEBUG
				shift
				;;

			"clang")
				USE_CLANG=1
				export USE_CLANG
				shift
				;;
			*)
				Z_BUILD_SCRIPT_PARAM+=("${1}")
				shift
				;;
	esac
done

