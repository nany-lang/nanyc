#/usr/bin/env bash


root=$(dirname "${0}")
list=$(find "${root}" -name '*.ny' -exec cat '{}' \; | grep -E -o '!![a-z_][a-z0-9_\.]*' | sort -u)

beginswith() { case $2 in "$1"*) true;; *) false;; esac; }

echo "OFFICIAL INTRINSICS"
for intrinsic in ${list}; do
	if ! beginswith "!!__" "$intrinsic"; then
		echo "    ${intrinsic}"
	fi
done

echo
echo
echo "VENDOR SPECIFIC INTRINSICS"
for intrinsic in ${list}; do
	if beginswith "!!__" "$intrinsic"; then
		echo "    ${intrinsic}"
	fi
done

echo
echo "total: $(echo "${list}" | wc -l)"
