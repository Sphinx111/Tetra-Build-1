# Receives data from decoder (C++), through port.

# Simple version of handler that ONLY tracks total number of radios active at
# any one time, and records the total on a database

# For each burst, save a note of the time the radio was last Seen.
# Every n time steps, create a Row in a simpleCalls table

# connects to a database
# Writes calls and sessions to database
# Writes any new IDs to database

import socket
import time
from threading import Thread
import datetime
import databaseHandler

localIP = "127.0.0.1"
localPort = 42100
bufferSize = 65535  # should match output databuffer size, from C++ elements
cl_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
cl_socket.bind((localIP, localPort))
dataBuffer = []
radiosDict = {}         # Mobile Stations (radios)
controllersList = ['16777215', '7179146']  # Control stations, ignore these
frequenciesList = [392.75, 393.28, 394.255]
radioTimeout = 300
updateFrequency = 60

db = databaseHandler.main()

print("socket created and ready")


# Loop to be threaded to receive the raw data from Decoder
def receiveRawData():
    while True:
        data = cl_socket.recv(bufferSize)

        # TODO: create a new break condition to call cl_socket.close()

        if data != "b'\n'":
            # print("RECEIVED: %s" % data)
            dataBuffer.append(data)


recv_thread = Thread(target=receiveRawData)
recv_thread.start()


# Start processing the dataBuffer, only handling speech and data frames
def processRawData():
    while 1:
        # if something in dataBuffer, process it, otherwise, sleep for 10ms
        if len(dataBuffer) > 0:
            # convert data to string, then split into array of each element
            thisLine = dataBuffer.pop(0).decode('utf8')
            thisLine = thisLine.split(',')
            if len(thisLine) > 8:
                if (thisLine[1] == '"pdu":"TCH_S"'):
                    processSpeechFrame(thisLine)
                if (thisLine[1] == '"pdu":"RAW-DATA"' and len(thisLine) > 12):
                    # print("Data Burst received!")
                    # print(thisLine[12])
                    time.sleep(0.01)
        else:
            time.sleep(0.1)


def processSpeechFrame(thisLine):
    thisRadio = thisLine[5].split(':')[1]
    timeOfBurst = float(thisLine[8].split(':')[1])
    if thisRadio != "0":
        try:
            if thisRadio not in controllersList:
                radiosDict.update({thisRadio: timeOfBurst})
        except Exception as e:
            print(e)


def cleanup():
    while 1:
        time.sleep(updateFrequency)
        cleanList = []
        updateTime = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        databaseHandler.addSimpleCount(updateTime, len(radiosDict))
        for r in radiosDict:
            timeDiff = round(time.time(), 0) - radiosDict[r]
            print("Debug: timeDiff for Radio " + r + " is " + str(timeDiff))
            if timeDiff > radioTimeout:
                cleanList.append(r)
        for r in cleanList:
            radiosDict.pop(r)
        print(str(len(radiosDict)) + " radios Active")


process_thread = Thread(target=processRawData)
process_thread.start()


cleanup_thread = Thread(target=cleanup)
cleanup_thread.start()
