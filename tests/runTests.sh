#!/bin/bash

TEST_SOURCES=$(find src -name *.cc)
NUM_TESTS=$(echo $TEST_SOURCES |wc -w)
YELLOW_TEXT="\e[01;33m"
GREEN_TEXT="\e[01;32m"
RED_TEXT="\e[01;31m"
RESET_TEXT="\e[01;00m"
NUM_SUCCEEDED=0
NUM_FAILED_TO_COMPILE=0
NUM_FAILED=0

i=0
for f in $TEST_SOURCES; do
	HEADER="Test $i / $NUM_TESTS: $f:"
	
	let i+=1
	echo -ne "$HEADER$YELLOW_TEXT COMPILING$RESET_TEXT"
	
	obj=$(echo $f |sed s/.cc// |sed s@src/@build/test_@)
	errs=$(echo $f |sed s/.cc// |sed s@src/@build/errors_@)
	out=$(echo $f |sed s/.cc// |sed s@src/@build/output_@)
	g++ $f -o $obj -I.. -L../build/libcobaltcore/ -lcobaltcore -std=c++11 2>$errs
	
	if [ "$?" != "0" ]; then
		echo -e "\r$HEADER$RED_TEXT FAILED TO COMPILE$RESET_TEXT"
		let NUM_FAILED_TO_COMPILE+=1
		continue
	fi
	
	echo -ne "\r$HEADER$YELLOW_TEXT RUNNING$RESET_TEXT     "
	$obj >$out 2>$out
	
	if [ "$?" != "0" ]; then
		echo -e "\r$HEADER$RED_TEXT FAILED$RESET_TEXT     "
		let NUM_FAILED+=1
		continue
	fi
	
	echo -e "\r$HEADER$GREEN_TEXT SUCCESS$RESET_TEXT     "
	let NUM_SUCCEEDED+=1
done

echo "$NUM_TESTS ran"
echo "$NUM_SUCCEEDED tests succeeded"
echo "$NUM_FAILED_TO_COMPILE tests failed to compile"
echo "$NUM_FAILED tests exited with an error code"
echo "Success rate: $(expr $NUM_SUCCEEDED \* 100 / $NUM_TESTS)%"