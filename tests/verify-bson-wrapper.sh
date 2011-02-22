#! /bin/sh
set -e

run_test ()
{
	test=$1
	no=$2
	t=$(mktemp)
	ret=1

	if test -x ./test_${test}; then
		if ./test_${test}; then
			bsondump libmongo-tests.bson | head -n -1 >${t}
			if cmp ${t} ${srcdir}/result_${test}; then
				ret=0
			fi
		fi
	fi
	rm -f libmongo-tests.bson "${t}"

	if test ${ret} = 0; then
		echo "ok ${no}"
	else
		echo "not ok ${no}"
	fi
}

if ! test -z "$(which bsondump)"; then
	echo "1..2"
	run_test bson_build_base 1
	run_test bson_build_compound 2
else
	echo "1..0 # SKIP bsondump not found"
fi
