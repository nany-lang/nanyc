#!/usr/bin/env bash
set -e

die() {
	echo
	echo
	echo "   *** failed: ${1}"
	exit 1
}

run() {
	local tag="$1"
	local title="$2"
	local ANSI_CLEAR="\033[0K"
	shift
	shift
	if [ -n "${TRAVIS_JOB_NUMBER:-}" ]; then
		echo -en "travis_fold:start:${tag}\r${ANSI_CLEAR}"
		echo "${title}"
	else
		echo -e "\n\n\n{{{ $title }}}\n"
	fi
	$@ || die "failed to execute '$@'"
	[ -n "${TRAVIS_JOB_NUMBER:-}" ] && echo -en "travis_fold:end:${tag}\r${ANSI_CLEAR}"
	return 0
}

platform_and_env_settings() {
	platform='unknown'
	unamestr=`uname`
	if [[ "$unamestr" == 'Linux' ]]; then
		platform='linux'
	elif [[ "$unamestr" == 'FreeBSD' ]]; then
		platform='freebsd'
	elif [[ "$unamestr" == 'Darwin' ]]; then
		platform='macos'
	fi
	echo "platform: ${platform}"

	export NPROC=1
	[ $platform == linux ] && export NPROC=$(nproc)
	[ $platform == macos ] && export NPROC=$(sysctl -n hw.ncpu)
	export platform="${platform}"
	[ -z "${BUILD_TYPE:-}" ] && export BUILD_TYPE="debug"

	uname -a
	cat	../build-settings.txt
}

compiler_settings() {
	[ -z "${CC:-}" ]  && export CC="gcc";
	[ -z "${CXX:-}" ] && export CXX="g++";
	if [ -n "${TRAVIS_JOB_NUMBER:-}" -a $platform == linux ]; then
		[ "$CC"  == "gcc" ] && export CC="gcc-5";
		[ "$CXX" == "g++" ] && export CXX="g++-5";
	fi
	echo "env CC=${CC} [$(which $CC)]"
	echo "env CXX=${CXX} [$(which $CXX)]"
}

cmake_cleanup() {
	echo "delete CMakeCache.txt" && rm -f CMakeCache.txt
	echo "delete CMakeFiles" && rm -rf CMakeFiles
}

configure() {
	echo "# cmake . -DCMAKE_BUILD_TYPE=${BUILD_TYPE}"
	cmake . -DCMAKE_BUILD_TYPE=${BUILD_TYPE} || die "bootstrap configure error"
}

build() {
	echo "# make -j ${NPROC}"
	make -j ${NPROC} || (run  "retry" "ERROR" make VERBOSE=1 ; exit 1) || die "build failed"
	make check || die "check failed"
}

make_packages_deb() {
	make packages-deb || die "packages deb failed";
}

print_packages() {
	(cd "${root}/../../distrib" && find  -maxdepth 1 -and -type f  -and -not -path '*/\.*' \
		| xargs  ls -lh)
}

make_packages() {
	[ ! "${BUILD_TYPE}" = 'release' ] && return 0
	local has_pkgs=0
	[ $platform == linux ] && run "DEB" "Packages DEB" make_packages_deb && has_pkgs=1
	[ $has_pkgs -ne 0 ] && run "output" "Distribution" print_packages
	return 0
}


local_pwd=$(dirname "$0")
root=$(cd "${local_pwd}" && pwd)
pushd "${root}/../../bootstrap"
[ -z "${BUILD_TYPE:-}" ] && export BUILD_TYPE="release"

run "settings" "Platform / Env Settings" platform_and_env_settings
run "compiler" "C++ Compiler settings" compiler_settings
[ -z "${TRAVIS_JOB_NUMBER:-}" ] && run "cleanup" "CMake cleanup" cmake_cleanup

echo

run "cmake" "Bootstrap: configure" configure
run "build" "Bootstrap: build" build
make_packages
