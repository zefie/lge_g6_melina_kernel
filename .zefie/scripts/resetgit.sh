#!/bin/bash
if [ $(ls -l AndroidKernel.mk | wc -l) -eq 1 ]; then
	if [ $(git status | grep clean | wc -l) -ne 1 ]; then
		if [ $(git status | grep "\.zefie/" | wc -l) -gt 0 ]; then
			TMPFILE=$(tempfile -p zef- -s .zef)
			tar -cf ${TMPFILE} .zefie/
		fi
		rm * .config .config.old -rf
		git reset --hard
		if [ ! -z "${TMPFILE}" ]; then
			rm -rf .zefie
			tar -xf ${TMPFILE}
			rm ${TMPFILE}
		fi
	fi
fi
