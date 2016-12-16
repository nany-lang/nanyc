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

local_pwd=$(dirname "$0")
root=$(cd "${local_pwd}" && pwd)
pushd "${root}/../../"


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

[ "$BUILD_TYPE" == "" ] && export BUILD_TYPE="debug"



# update on TRAVIS
if [ -n "${TRAVIS_OS_NAME}" -a $platform == linux ]; then
	[ "$CC"  == "" ]    && export CC="gcc-4.9";
	[ "$CXX" == "" ]    && export CXX="g++-4.9";
	[ "$CC"  == "gcc" ] && export CC="gcc-4.9";
	[ "$CXX" == "g++" ] && export CXX="g++-4.9";
fi
[ -z "$CC" ]  && export CC="gcc";
[ -z "$CXX" ] && export CXX="g++";


title "ENVIRONMENT"
cat ./build-settings.txt
echo
env


title "CLEANUP"
cd bootstrap
echo " - delete CMakeCache.txt" && rm -f CMakeCache.txt
echo " - delete CMakeFiles" && rm -rf CMakeFiles


title "CONFIGURE"
echo "# cmake . -DCMAKE_BUILD_TYPE=${BUILD_TYPE}"
cmake . -DCMAKE_BUILD_TYPE=${BUILD_TYPE} || die "bootstrap configure error"


title "BUILD"
echo "# make -j ${NPROC}"
make -j ${NPROC} || (title "MAKE ERROR"; make VERBOSE=1 ; exit 1) || die "build failed"
make check || die "check failed"


if [ $platform == linux ]; then
	title "PACKAGE DEB"
	make packages-deb || die "packages deb failed";
fi


title "OUTPUT"
ls -lh "${root}/../../distrib"
