#!/bin/bash
function try_patch() {
	local z_res=0;
	if [ ! -z "${1}" ]; then
		git cherry-pick "${1}"
		z_res=$?
		if [ "${z_res}" -ne 0 ]; then
			if [ -z "${2}" ]; then
				echo "*** Aborting... specify anything after the commit hash to prevent aborting"
				git cherry-pick --abort
			else
				echo "*** Warning, cherry-pick was NOT aborted. Please fix up or manually abort it."
			fi
		fi
	fi
}

if [ ! -z "${1}" ]; then
	try_patch "${@}"
fi

