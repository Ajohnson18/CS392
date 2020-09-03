###############################################################################
# Author: Alex Johnson
# Date: 2/4/2020
# Pledge: I pledge my honor that I have abided by the Stevens Honor System.
# Description: Creates a recycling bin to move and destroy files.
###############################################################################
#!/bin/bash

readonly RECYCLE_BIN=".junk"
list_flag=0
purge_flag=0
help_flag=0
fbname=$(basename "$0")

sendMessage() {
	local base=$1
	cat << ENDOFTEXT
Usage: $fbname [-hlp] [list of files]
 -h: Display help.
 -l: List junked files.
 -p: Purge all files.
 [list of files] with no other arguments to junk those files.
ENDOFTEXT
}

if [ $# -eq 0 ]; then
	sendMessage
fi

while getopts ":hlp" option; do
	case "$option" in
		h) help_flag=1
			;;
		l) list_flag=1
			;;
		p) purge_flag=1
			;;
		?) printf "Error: Unknown option '-%s'.\n" $OPTARG >&2
			sendMessage
			exit 1
			;;
	esac
done

if [ ! -d "../$RECYCLE_BIN" ]; then
	cd ~
	mkdir "$RECYCLE_BIN"
fi

if [ $# -gt 1 ] && ([ $help_flag -eq 1 ] || [ $list_flag -eq 1 ] || [ $purge_flag -eq 1 ]); then
	printf "Error: Too many options enabled.\n"
	sendMessage
	exit 1
fi

if [ $help_flag -eq 1 ]; then
	sendMessage
	exit 0
fi

if [ $list_flag -eq 1 ]; then
	cd ../.junk
	ls -lAF
	exit 0
fi

if [ $purge_flag -eq 1 ]; then
	cd ~
	rm -r "$RECYCLE_BIN"
	mkdir "$RECYCLE_BIN"
	exit 0
fi

if [ $list_flag -eq 0 ] && [ $help_flag -eq 0 ] && [ $purge_flag -eq 0 ]; then
	for var in "$@"; do
		if [ -f "$var" ] || [ -d "$var" ]; then
			mv "$var" "../$RECYCLE_BIN"
		elif [ ! -f "$var" ] || [ ! -d "$var" ]; then
			echo "Warning: '$var' not found."
		fi
	done
	exit 0
fi

