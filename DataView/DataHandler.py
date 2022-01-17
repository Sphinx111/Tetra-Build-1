# Receives data from decoder (C++), through port.

# Holds a list of very recent bursts in memory
# If no similar bursts for time t, classifies those bursts as a "call"
# a "call" is created, and bursts are cleared from memory
# a "call" contains: start ID, receiving IDs, duration, startTime, finishTime
# and any special flags

# Holds a list of recent calls in memory
# If no similar calls for time t, creates a new "session" object
# If there's an existing session for these calls
# If there were similar calls recently, adds the call to a "session"
# a "session" contains: IDs involved, startTime, duration, finishTime, calls
# and any special flags.

# connects to a database
# Writes calls and sessions to database
# Writes any new IDs to database

import socket
import time
from threading import Thread
import itertools
import databaseHandler
# import networkx as nx

localIP = "127.0.0.1"
localPort = 42100
bufferSize = 65535  # should match output databuffer size, from C++ elements
cl_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
cl_socket.bind((localIP, localPort))
dataBuffer = []
callSeparationTime = 5  # seconds paus allowed before classifying as a new call
sessionSeparationTime = 20  # ------------before classifying as a new session
radiosDict = {}
callsList = []
sessionsList = []

db = databaseHandler.main()

# G = nx.Graph()

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
                    print("Data Burst received!")
                    print(thisLine[12])
        else:
            time.sleep(0.1)


def processSpeechFrame(thisLine):
    thisRadio = thisLine[5].split(':')[1]
    timeOfBurst = thisLine[8].split(':')[1]
    print(thisRadio + " sent burst at " + timeOfBurst)
    if thisRadio != "0":
        try:
            thisCall = find_active_call(thisRadio, timeOfBurst)
            if thisCall not in callsList:
                callsList.append(thisCall)
            thisSession = find_active_session(thisCall)
            if thisSession not in sessionsList:
                sessionsList.append(thisSession)
            radiosDict.update({thisRadio: timeOfBurst})
        except Exception as e:
            print(e.stackTrace())
    print(str(len(radiosDict)) + " radios Active - " + str(radiosDict.keys()))
    print(str(len(callsList)) + " calls active")
    print(str(len(sessionsList)) + " sessions active")


process_thread = Thread(target=processRawData)
process_thread.start()


# the Call object holds a list of radios and key info about the call
class Call():
    id = 0
    isEmergency = 0
    start_time = 0
    end_time = 0
    last_frame = 0
    isClosed = True
    newid = next(itertools.count())
    radios = []
    session = 0

    def __init__(self, newRadio, updateTime):
        self.id = Call.newid
        self.radios.append(newRadio)
        self.start_time = updateTime
        self.isClosed = False

    def end_call(self, endTime):
        self.end_time = endTime
        databaseHandler.addCallToDB(self.id,
                                    self.start_time,
                                    self.end_time,
                                    self.radio)
        databaseHandler.addRadioToDB(self.radio, self.end_time)
        self.isClosed = True
        if self.session == 0:
            activeSession = find_active_session(self)
            activeSession.add_call_to_session(self)

    def setEmergency(self, value):
        self.isEmergency = value


class Session():
    id = 0
    calls = []
    start_time = 0
    end_time = 0
    last_call_time = 0
    isClosed = False

    def __init__(self, firstCall):
        Session.id += 1
        self.id = Session.id
        self.calls.append(firstCall)
        self.start_time = firstCall.start_time
        self.last_call_time = firstCall.end_time

    def end_session(self):
        self.end_time = self.last_call_time
        databaseHandler.addSessionToDB(self.id,
                                       self.start_time,
                                       self.end_time,
                                       self.radio)
        self.isClosed = True

    # Only call this when a new call has finished
    def add_call_to_session(self, newCall):
        newCall.session = self
        self.calls.append(newCall)
        self.last_call_time = newCall.end_time


def find_active_session(testCall):
    for s in sessionsList:
        if not s.isClosed:
            for c in s.calls:
                for r in c.radios:
                    for r2 in testCall.radios:
                        if r == r2:
                            t = float(testCall.end_time) - float(c.end_time)
                            if t < sessionSeparationTime:
                                return s
                            else:
                                # if the radios Match, but it's been too long
                                # close the session being tested
                                s.isClosed = True
    # If no active session found for the radio on this call, create new and rtn
    return Session(testCall)


def find_active_call(testRadio, testTime):
    for c in callsList:
        if not c.isClosed:
            for r in c.radios:
                if testRadio == r:
                    timeDiff = float(testTime) - float(radiosDict[r])
                    print("DEBUG " + str(timeDiff) + " timeDiff for call")
                    if timeDiff < callSeparationTime:
                        return c
    # if there's no active call found, create a new call and return it
    return Call(testRadio, testTime)


time.sleep(20)
# G.add_nodes_from(callsList)
