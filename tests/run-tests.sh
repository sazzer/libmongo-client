#! /bin/sh
set -e

run_test ()
{
	test=$1
	t=$(tempfile)

	echo "# Running ${test}..."
	if ! ./test_${test} >$t 2>/dev/null; then
		rm -f $t
		return 1
	fi
	if ! cmp $t ${srcdir}/${test}.ok 2>/dev/null; then
		rm -f $t
		return 1
	fi

	rm -f $t
	return 0
}

run_test bson_build_simple
