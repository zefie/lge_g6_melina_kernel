#!/bin/bash
if [ -d ".zefie" ]; then
	if [ "$(git status | grep "modified:" | grep -v "\.zefie/" | wc -l)" -gt "0" ]; then
		# If files other than those in .zefie have been modified, we shall reset
		if [ "$(git status | grep "\.zefie/" | wc -l)" -gt "0" ]; then
			# If we are resetting, and files in .zefie have been modified, we shall retain them
			TMPFILE=$(tempfile -p zef- -s .zef)
			echo "* Saving current status of zefie scripts..."
			tar -cf ${TMPFILE} .zefie/
		fi
		rm * .config .config.old -rf
		git reset --hard
		if [ ! -z "${TMPFILE}" ]; then
			rm -rf .zefie
			echo "* Restoring pre-reset status of zefie scripts..."
			tar -xf ${TMPFILE}
			rm ${TMPFILE}
		fi
	fi
fi
