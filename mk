#!/bin/bash

function make()
{
	local ret=0
	local target
	if (( "$#" == 0 )) ; then
		target="compile"
		if ! /usr/bin/time -f "%E (%P)" make "$target" 2>&1 ; then
			ret=1
		fi
	else
		for target in "$@" ; do
			if ! /usr/bin/time -f "%E (%P)" make "$target" 2>&1 ; then
				ret=1
			fi
		done
	fi
	return "$ret"
}

function main()
{
	clear
	local irun=
	local target=
	local run="false"
	for (( irun = 1; irun <= $#; irun++ )) ; do
		target="${@:$irun:1}"
		if [ "$target" = "run" ] ; then
			run="true"
			break
		fi
	done
	# echo "run: $run, irun: $irun, runcmds: ${@:$irun+1}, make: ${@:1:$irun}"
	if [ "$run" = "true" ] ; then
		out="$(make "${@:1:$irun}")"
	else
		out="$(make "$@")"
	fi
	local ret="$?"
	if (( "$(echo "$out" | wc -l)" > "$(tput lines)" - 1 )) ; then
		echo "$out" | less -R
	else
		echo "$out"
	fi
	
	if (( "$ret" == 0 )) && [ "$run" = "true" ] ; then
		local exec="$(echo "$out" | sed -n -e 's/^run: \(.*\)/\1/p')"
		if (( "$(echo "$exec" | wc -l)" == 1 )) ; then
			echo
			echo "executing $exec ${@:$irun+1}"
			echo
			/usr/bin/time -f "%E" "$exec" "${@:$irun+1}"
		fi
	fi
	
	return "$ret"
}

( cd "$(dirname "$(realpath "${BASH_SOURCE[0]}")")" && main "$@" )





