#!/bin/bash
# shellcheck disable=SC1090

#export USE_CCACHE=1

KERNEL_DEVMODEL="${KERNEL_DEVMODEL:-US997}"
export KERNEL_DEVMODEL

SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"


errchk cd "${KERNEL_SOURCE_DIR}"

Z_CONFIG_OVERRIDE=()

Z_SUPPORTED_CMDS=($(grep ')$' "${SCRIPTDIR}/build.sh" | grep -v '(' | grep -v "\*" | grep -v "\-\-" | cut -d'"' -f2))
Z_SUPPORTED_CMDS+=(clang noclang debug nodebug)
Z_INTERNAL_FUNCS=($(grep function "${SCRIPTDIR}/functions.sh" | cut -d'(' -f1 | rev | cut -d' ' -f1 | rev | grep -v '{'))

while [ "${1}" != "" ]; do
                case "${1}" in

			"setdevice")
				shift
				if [ -z "${1}" ]; then
					echo "Usage: setdevice devicename"
					exit 1;
				else
					kernel_setdevice "${1}"
				fi
				shift;
				;;

			"defconfig")
				shift
				errchk kernel_create_defconfigs "${Z_CONFIG_OVERRIDE[@]}"
				errchk kernel_defconfig
				;;

			"clean")
				shift
				errchk kernel_clean
				;;

			"gitreset")
				shift
				errchk kernel_git_reset
				errchk kernel_clean
				;;

			"debug"|"nodebug")
				USE_DEBUG=$(if [ "${1}" == "debug" ]; then echo 1; fi)
				export USE_DEBUG
				shift
				;;

			"clang"|"noclang")
				USE_CLANG=$(if [ "${1}" == "clang" ]; then echo 1; fi)
				export USE_CLANG
				shift
				;;

			"build")
				shift
				kernel_release_build
				;;

			"release-build")
				shift
				kernel_setdevice us997
				kernel_release_build

				kernel_setdevice h870
				kernel_release_build

				kernel_setdevice h872
				kernel_release_build

				export USE_DEBUG=1
				kernel_setdevice us997
				kernel_release_build

				kernel_setdevice h870
				kernel_release_build

				kernel_setdevice h872
				kernel_release_build
				unset USE_DEBUG
				;;

			"make")
				shift
				make "${@}"
				exit $?
				;;
			"shell")
				shift
				"${@}"
				exit $?
				;;

			"zip")
				shift
				errchk "${SCRIPTDIR}/buildzip.sh"
				;;

			"help")
				echo "The following arguments to ${0} are available:"
				echo "${Z_SUPPORTED_CMDS[*]}"
				if [ ! -z "${Z_SHELL_ENV}" ]; then
					echo "Since you are in the melina-dev environment, the following functions are available via the cmdline:"
					echo "${Z_INTERNAL_FUNCS[*]}"
				fi
				break;
				;;
			"--")
				continue;
				;;
			*)
				Z_BUILD_SCRIPT_PARAM+=("${1}")
				shift
				;;
	esac
done

