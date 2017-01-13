#!/usr/bin/env bash
set -e
imgtag="$1"

image="nanyc/build:$imgtag"
root=$(cd . && pwd)

echo "::: running ${image} | ${BUILD:-release}"

docker pull "${image}"
docker run -t -v "${root}:/mnt/build/nanyc" -w "/mnt/build/nanyc" \
	--env BUILD_TYPE="${BUILD:-release}" \
	--env TRAVIS="${TRAVIS:-}" \
	--env TRAVIS_TAG="${TRAVIS_TAG:-}" \
	--env TRAVIS_JOB_NUMBER="${TRAVIS_JOB_NUMBER:-}" \
	--env TRAVIS_PULL_REQUEST="${TRAVIS_PULL_REQUEST:-}" \
	"${image}" bash -c ./assets/build/build.sh
