#!/bin/bash
#
# 2020-07-19  LT  0.0  first release
#

for FILE in *.raw; do
    BASE=${FILE%.raw};
    echo "* $BASE.raw -> $BASE.wav";

    sox -r 8k -e signed -b 16 "$BASE.raw" "$BASE.wav"
done
