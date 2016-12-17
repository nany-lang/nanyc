#!/usr/bin/env bash


title() {
	echo
	echo
	echo
	echo
	echo "--- [ $1 ] ---"
	echo
}

die() {
	echo
	echo
	echo "   *** failed: ${1}"
	exit 1
}

run() {
	title $1; shift
	$@
}

platform_and_env_settings() {
	title "GENERAL"
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
	[ "$BUILD_TYPE" == "" ] && export BUILD_TYPE="debug"

	uname -a
	cat	../build-settings.txt
}

compiler_settings() {
	if [ -n "${TRAVIS_JOB_NUMBER}" -a $platform == linux ]; then
		[ "$CC"  == "" ]    && export CC="gcc-5";
		[ "$CXX" == "" ]    && export CXX="g++-5";
		[ "$CC"  == "gcc" ] && export CC="gcc-5";
		[ "$CXX" == "g++" ] && export CXX="g++-5";
	fi
	[ -z "$CC" ]  && export CC="gcc";
	[ -z "$CXX" ] && export CXX="g++";
}

cmake_cleanup() {
	cd bootstrap
	echo " - delete CMakeCache.txt" && rm -f CMakeCache.txt
	echo " - delete CMakeFiles" && rm -rf CMakeFiles
}

configure() {
	echo "# cmake . -DCMAKE_BUILD_TYPE=${BUILD_TYPE}"
	cmake . -DCMAKE_BUILD_TYPE=${BUILD_TYPE} || die "bootstrap configure error"
}

build() {
	echo "# make -j ${NPROC}"
	make -j ${NPROC} || (run  "SOMETHING HAS FAILED" make VERBOSE=1 ; exit 1) || die "build failed"
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
	[ ! "${BUILD_TYPE:-}" = 'release' ] && return
	local has_pkgs=0
	[ $platform == linux ] && run "Packages DEB" make_packages_deb && has_pkgs=1
	[ $has_pkgs -ne 0 ] && run "Distribution" print_packages
	return 0
}


local_pwd=$(dirname "$0")
root=$(cd "${local_pwd}" && pwd)
pushd "${root}/../../bootstrap"

run "Platform & Env settings" platform_and_env_settings
run "C++ Compiler settings" compiler_settings
[ -z "${TRAVIS_JOB_NUMBER}" ] && cmake_cleanup

run "Bootstrap: configure" configure
run "Bootstrap: build" build
make_packages
