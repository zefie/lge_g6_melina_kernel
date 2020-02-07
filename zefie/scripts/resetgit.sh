#!/bin/bash
source .zefie/scripts/buildenv.sh

function z_reset_git {
	echo "Resetting source tree..."
	for f in $(ls . | grep -v "^${KERNEL_BUILDDIR}$"); do
		rm -rf "${f}"
	done;
	rm -f .config .config.old
	rm -rf "${KERNEL_BUILDDIR:?}/"*
	git reset --hard
	exit $?
}

if [ -d ".git" ]; then
	if [ ! -z "${FORCE_RESET}" ]; then
		z_reset_git
	fi

	if git diff-index --name-only HEAD | grep -qv "^scripts/package"; then
		if git diff-index --name-only HEAD | grep -qv "^.zefie"; then
			z_reset_git
		fi
	fi
fi
