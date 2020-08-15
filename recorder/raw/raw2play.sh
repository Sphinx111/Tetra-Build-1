#!/bin/bash
# 2020-08-13 Original source code from @orestescaminha
# 2020-08-13 Additional notes by Larry "chmod u+rwx raw2play.sh" should be enough, no root access required
#
# Save this in recorder/raw dir, "chmod 777 raw2play.sh" and run as ./raw2play.sh
# Needs "apt-get install inotify-tools" if it is not already installed
# Run recorder using -a flag
#
# get the current path (recorder/raw) and wav path (recorder/wav)
CURPATH=`pwd`
WAVPATH=../wav
#echo $CURPATH
#echo $WAVPATH
while FILERAW=$(inotifywait -e create $CURPATH --format %f) 
	do
	while true
	    do       #Tests whether the .raw file is finished
		    A=$(stat -c %s $CURPATH/$FILERAW)
		    #echo $A Bytes is the size of $FILERAW
	        sleep 2
            B=$(stat -c %s $CURPATH/$FILERAW)
            #echo $B Bytes is is the size of $FILERAW after 2s
    	    DIFF=$((B-A))
            #echo $DIFF Bytes is the diference
		    
			if [ "$DIFF" == 0 ]; then
                echo "!!!!!!!!!!! .raw file completed !!!!!!!!!!!"
				break  # Returns to the beginning of the loop if .raw file has not completed
		    else
			    echo "==== Waiting for .raw file completion ===="
			fi
		done
		
        for FILE in $FILERAW; do
            BASE=$(echo "$FILE" | cut -f 1 -d '.');	 # piping cut to remove the extension .raw
			echo "$BASE -> $WAVPATH/$BASE.wav" >/dev/null 2>&1 &
            sox -r 8k -e signed -b 16 "$BASE.raw" "$WAVPATH/$BASE.wav"
			echo "Listening to $BASE.wav ($B Bytes)"
			play $WAVPATH/$BASE.wav >/dev/null 2>&1 &
	    done
    done