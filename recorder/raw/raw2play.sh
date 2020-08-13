#!/bin/bash
# 2020-08-13 Original source code from @orestescaminha
# 2020-08-13 Additional notes by Larry "chmod u+rwx raw2play.sh" should be enough, no root access required
#
# Save this in recorder/wav dir, "chmod 777 raw2play.sh" and run as ./out2play.sh
# Needs inotify-tools if it is not already installed
# Run recorder using -a flag
#
# get the current path and raw path
CURPATH=`pwd`
TARGET=../raw
#echo $CURPATH
#echo $TARGET
while FILERAW=$(inotifywait -e create $TARGET --format %f) 
	do
	while true
	    do       #Tests whether the .raw file is finished
		    A=$(stat -c %s $TARGET/$FILERAW)
		    #echo $A Bytes is the size of $FILERAW
	        sleep 2
            B=$(stat -c %s $TARGET/$FILERAW)
            #echo $B Bytes is is the size of $FILERAW after 2s
    	    DIFF=$((B-A))
            #echo $DIFF Bytes is the diference
		    
			if [ "$DIFF" == 0 ]; then
                echo "====Speech completed===="
				break  # Returns to the beginning of the loop if the conversation has not ended
		    else
			    echo "==== Waiting for speech completion ===="
			fi
		done
		
	cp -rf $TARGET/$FILERAW $CURPATH
        for FILE in $CURPATH/$FILERAW; do
            BASE=$(echo "$FILE" | cut -f 1 -d '.');	 # piping cut to remove the extension .out
            echo "$FILE -> $BASE.wav" >/dev/null 2>&1 &

			sox -r 8k -e signed -b 16 "$BASE.raw" "$BASE.wav"
			echo Listen to $BASE.wav
			play "$BASE.wav" >/dev/null 2>&1 &
	    done
    rm  *.raw
    done
