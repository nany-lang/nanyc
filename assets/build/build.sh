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
	cat	./build-settings.txt
}

compiler_settings() {
	if [ -n "${TRAVIS_OS_NAME}" -a $platform == linux ]; then
		[ "$CC"  == "" ]    && export CC="gcc-4.9";
		[ "$CXX" == "" ]    && export CXX="g++-4.9";
		[ "$CC"  == "gcc" ] && export CC="gcc-4.9";
		[ "$CXX" == "g++" ] && export CXX="g++-4.9";
	fi
	[ -z "$CC" ]  && export CC="gcc";
	[ -z "$CXX" ] && export CXX="g++";
}

cmake_cleanup() {
	title "CLEANUP"
	cd bootstrap
	echo " - delete CMakeCache.txt" && rm -f CMakeCache.txt
	echo " - delete CMakeFiles" && rm -rf CMakeFiles
}

configure() {
	title "CONFIGURE"
	echo "# cmake . -DCMAKE_BUILD_TYPE=${BUILD_TYPE}"
	cmake . -DCMAKE_BUILD_TYPE=${BUILD_TYPE} || die "bootstrap configure error"
}

build() {
	title "BUILD"
	echo "# make -j ${NPROC}"
	make -j ${NPROC} || (title "MAKE ERROR"; make VERBOSE=1 ; exit 1) || die "build failed"
	make check || die "check failed"
}

make_packages_deb() {
	title "PACKAGES DEB"
	make packages-deb || die "packages deb failed";
}

print_packages() {
	title "FOR DISTRIBUTION"
	(cd "${root}/../../distrib" && find  -maxdepth 1 -and -type f  -and -not -path '*/\.*' \
		| xargs  ls -lh)
}

make_packages() {
	if [ ! "${BUILD_TYPE:-}" = 'release' ]; then
		return
	fi
	local has_pkgs=0
	[ $platform == linux ] && make_packages_deb && has_pkgs=1
	[ $has_pkgs -ne 0 ] && print_packages
	return 0
}


local_pwd=$(dirname "$0")
root=$(cd "${local_pwd}" && pwd)
pushd "${root}/../../"

platform_and_env_settings
compiler_settings
[ -z "${TRAVIS_JOB_NUMBER}" ] && cmake_cleanup

configure
build
make_packages
