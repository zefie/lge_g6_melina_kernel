#!/bin/bash
if [ -f "${1}" ]; then
	.zefie/scripts/resetgit.sh
	for f in $(cat "${1}" | grep "\-\-\-" | cut -d' ' -f2 | cut -f1 | cut -d'/' -f2-); do
		DIR="$(echo ${f} | rev | cut -d'/' -f2- | rev)"
		mkdir -p "a/${DIR}"
		cp "${f}" "a/${DIR}/"
	done;
	cp -r a b
	cd b
	patch -p1 < "../${1}"
else
	echo "Creates a b folders with existing patch data, ready to edit"
	echo "Usage: ${0} patchfile"
fi
