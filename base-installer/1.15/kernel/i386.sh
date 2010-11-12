arch_get_kernel_flavour () {
	VENDOR=`grep '^vendor_id' /proc/cpuinfo | cut -d: -f2`
	FAMILY=`grep '^cpu family' /proc/cpuinfo | cut -d: -f2`
	case "$VENDOR" in
		" AuthenticAMD"*)
			case "$FAMILY" in
				" 6")	echo k7 ;;
				" 5")	echo k6 ;;
				*)	echo 386 ;;
			esac
		;;
		" GenuineIntel"*)
			case "$FAMILY" in
				" 6"|" 15")	echo 686 ;;
				" 5")		echo 586tsc ;;
				*)		echo 386 ;;
			esac
		;;
		*) echo 386 ;;
	esac
	return 0
}

arch_check_usable_kernel () {
	if expr "$1" : '.*-386.*' >/dev/null; then return 0; fi
	if [ "$2" = 386 ]; then return 1; fi
	if expr "$1" : '.*-586.*' >/dev/null; then return 0; fi
	if [ "$2" = 586tsc ]; then return 1; fi
	if expr "$1" : '.*-686.*' >/dev/null; then return 0; fi
	if [ "$2" = 686 ]; then return 1; fi
	if expr "$1" : '.*-k6.*' >/dev/null; then return 0; fi
	if [ "$2" = k6 ]; then return 1; fi
	if expr "$1" : '.*-k7.*' >/dev/null; then return 0; fi

	# default to usable in case of strangeness
	warning "Unknown kernel usability: $1 / $2"
	return 0
}

arch_get_kernel () {
	if [ -e /proc/speakup ]; then
		# Override and use speakup kernel. There's only one.
		echo "kernel-image-$KERNEL_VERSION-speakup"
		return
	fi

	if dmesg | grep -q ^Processors:; then
		SMP=-smp
	else
		SMP=
	fi
	if [ "$KERNEL_MAJOR" != 2.4 ]; then
		case "$1" in
			k6|586tsc)	set 386 ;;
		esac
	fi
	case "$1" in
		k7)	echo "kernel-image-$KERNEL_MAJOR-k7$SMP" ;;
		k6)	echo "kernel-image-$KERNEL_MAJOR-k6" ;;
		686)	echo "kernel-image-$KERNEL_MAJOR-686$SMP" ;;
		586tsc)	echo "kernel-image-$KERNEL_MAJOR-586tsc" ;;
		*)	echo "kernel-image-$KERNEL_MAJOR-386" ;;
	esac
}