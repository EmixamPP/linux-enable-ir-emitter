#compdef linux-enable-ir-emitter

_linux-enable-ir-emitter() {
	local -a opts
	local curcontext="$curcontext" state line
	typeset -A opt_args

	_arguments -C \
		'-v[print verbose information]' '--verbose[print verbose information]' \
		'-d[specify camera]:device:->devices' '--device[specify camera]:device:->devices' \
		'-w[specify width]:width:_guard "[0-9]#" "width"' '--width[specify width]:width:_guard "[0-9]#" "width"' \
		'-t[specify height]:height:_guard "[0-9]#" "height"' '--height[specify height]:height:_guard "[0-9]#" "height"' \
		'1: :->level1' \
		'*:: :->level2' && return 0

	case $state in
		devices)
			opts=($(ls /dev/video*))
			_describe 'devices' opts
			;;
		level1)
			opts=("run" "configure" "tweak" "test" "boot")
			_describe 'commands' opts
			;;
		level2)
			case $line[1] in
				configure)
					opts=("-m:manual verification" "--manual:manual verification" \
                    "-e:number of emitters" "--emitters:number of emitters" \
                    "-l:specify negative answer limit" "--limit:specify negative answer limit"\
                    "-g:disable video feedback" "--no-gui:disable video feedback" \
                    )
					_describe 'configure' opts
					;;
			esac
			;;
	esac
}

_linux-enable-ir-emitter
