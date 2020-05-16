#!/bin/bash
#
# 2016-07-21  LT  0.0  first release
#

cp -f ../out/*.out .

for FILE in *.out; do
    BASE=${FILE%.out};
    echo "* $FILE -> $BASE.wav";

    ./cdecoder "$FILE" "$BASE.cod";
    ./sdecoder "$BASE.cod" "$BASE.raw";
    sox -r 8k -e signed -b 16 "$BASE.raw" "$BASE.wav"
    #oggenc "$BASE.wav"
done

rm  *.out *.cod *.raw 
