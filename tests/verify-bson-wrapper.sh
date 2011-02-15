#! /bin/sh
set -e

run_test ()
{
	test=$1

	if test -x ./test_${test}; then
		if ! ./test_${test} | python ${srcdir}/verify-bson.py ${test}; then
			return 1
		fi
	else
		if ! python ${srcdir}/verify-bson.py ${test}; then
			return 1
		fi
	fi

	return 0
}

if run_test plan; then
	run_test bson_build_base
	run_test bson_build_compound
fi
