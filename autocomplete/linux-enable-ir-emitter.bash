#!/bin/bash
# Autocomplete file for bash

_linux-enable-ir-emitter() {
	local cur prev opts
	COMPREPLY=()
	cur="${COMP_WORDS[COMP_CWORD]}"
	prev="${COMP_WORDS[COMP_CWORD-1]}"

	case "${prev}" in
		"linux-enable-ir-emitter"|"-v"|"--verbose"|/dev/video*|"-w"|"--width"|"-t"|"--height")
			opts="-v --verbose -d --device -w --width -t --height run configure tweak test boot"
			;;
		"-d"|"--device")
            opts="$(ls /dev/video*)"
			;;
		"configure"|"-m"|"--manual"|"-e"|"--emitters"|"-l"|"--limit"|"-g"|"--no-gui")
			opts="-m --manual -e --emitters -l --limit -g --no-gui"
			;;
		"boot")
			opts="enable disable status"
			;;
 		*)
		return 0;
	esac

    COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
	return 0
}

complete -F _linux-enable-ir-emitter linux-enable-ir-emitter