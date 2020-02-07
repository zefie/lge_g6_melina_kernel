#!/bin/bash
# shellcheck disable=SC1090
SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"

function z_reset_git {
	echo "Resetting source tree..."
	rm -f .config .config.old
	rm -rf "${KERNEL_BUILD_DIR:?}/"*
	git reset --hard
	exit $?
}

if [ -d ".git" ]; then
	if [ ! -z "${FORCE_RESET}" ]; then
		z_reset_git
	fi

	if git diff-index --name-only HEAD | grep -qv "^scripts/package"; then
		if git diff-index --name-only HEAD | grep -qv "^zefie"; then
			z_reset_git
		fi
	fi
fi
