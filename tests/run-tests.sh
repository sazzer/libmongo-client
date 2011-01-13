#! /bin/sh
set -e

run_test ()
{
	test=$1

	if ! ./test_${test} | python ${srcdir}/verify-bson.py ${test}; then
		return 1
	fi

	rm -f $t
	return 0
}

run_test bson_build_base
run_test bson_build_compound
