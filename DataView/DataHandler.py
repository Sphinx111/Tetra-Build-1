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
from uuid import uuid4
import databaseHandler
# import networkx as nx

localIP = "127.0.0.1"
localPort = 42100
bufferSize = 65535  # should match output databuffer size, from C++ elements
cl_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
cl_socket.bind((localIP, localPort))
dataBuffer = []
callSeparationTime = 3  # seconds paus allowed before classifying as a new call
sessionSeparationTime = 20  # ------------before classifying as a new session
radiosDict = {}
callsList = []
sessionsList = []
callsCleanupList = []
sessionsCleanupList = []
lastFrameTime = 0
uidCounter = 0

db = databaseHandler.main()

# G = nx.Graph()

print("socket created and ready")

# get the new uidCounter Value from db
uidCounter = databaseHandler.resumeUniqueIDs()


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

    # Only remove calls/sessions from the list after processing all else
    if (len(callsCleanupList) > 0):
        for c in callsCleanupList:
            callsList.remove(c)
        callsCleanupList.clear()
    if (len(sessionsCleanupList) > 0):
        for s in sessionsCleanupList:
            sessionsList.remove(s)
        sessionsCleanupList.clear()


# If a frame is a Speech Frame break it down to the info we want
def processSpeechFrame(thisLine):
    thisRadio = int(thisLine[5].split(':')[1])
    timeOfBurst = float(thisLine[8].split(':')[1])
    timeLabel = (thisLine[8].split(':')[0])
    usageLabel = (thisLine[9].split(':')[0])
    if ((timeLabel != '"sysTime"') | (usageLabel != '"downlink usage marker"')):
        return
    usageMarker = int(thisLine[9].split(':')[1])
    if thisRadio != "0":
        try:
            thisCall = find_active_call(thisRadio, usageMarker, timeOfBurst)
            if thisCall not in callsList:
                callsList.append(thisCall)
            thisCall.updateTime(timeOfBurst)

            thisSession = find_active_session(thisCall)
            if thisSession not in sessionsList:
                sessionsList.append(thisSession)

            radiosDict.update({thisRadio: timeOfBurst})
            global lastFrameTime
            lastFrameTime = timeOfBurst
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
    isClosed = False
    usageMarker = -1
    radio = 0
    sessionID = 0

    def __init__(self, newRadio, usageMarker, startTime):
        global uidCounter
        self.id = uidCounter
        self.radio = newRadio
        self.start_time = startTime
        self.end_time = startTime
        self.usageMarker = usageMarker
        self.isClosed = False
        uidCounter += 1

    def __eq__(self, other):
        if (self.id == other.id):
            return True
        else:
            return False

    def end_call(self):
        databaseHandler.addRadioToDB(self.radio, self.end_time)
        if (self.sessionID == 0):
            activeSession = find_active_session(self)
            activeSession.add_call_to_session(self)
        databaseHandler.addCallToDB(self.id,
                                    self.isEmergency,
                                    self.start_time,
                                    self.end_time,
                                    self.radio,
                                    self.sessionID)
        self.isClosed = True

    def setEmergency(self, value):
        self.isEmergency = value

    def isFrameInThisCall(self, testRadio, testMarker):
        if (self.isClosed):
            return False
        if (self.radio != testRadio):
            return False
        if (self.usageMarker == testMarker):
            return True
        usageDiff = abs(self.usageMarker - testMarker)
        if (usageDiff > 6):
            return False
        return True

    def updateTime(self, newTime):
        self.end_time = newTime


class Session():
    id = 0
    calls = []
    start_time = 0
    end_time = 0
    isClosed = False

    def __init__(self, firstCall):
        global uidCounter
        self.id = uidCounter
        self.calls.append(firstCall)
        self.start_time = firstCall.start_time
        self.end_time = firstCall.end_time
        uidCounter += 1

    def __eq__(self, other):
        if (self.id == other.id):
            return True
        else:
            return False

    def end_session(self):
        databaseHandler.addSessionToDB(self.id,
                                       self.start_time,
                                       self.end_time,
                                       self.radio)
        self.isClosed = True

    # returns a weighting of how likely a call is to be in this session.
    def isCallInThisSession(self, testCall):
        minTime = sessionSeparationTime
        for c in self.calls:
            # if the testCall overlapped with any existing calls in session
            # it can't be part of this session, immediately return 0
            if (doCallsOverlap(c, testCall)):
                return 0
            timeSep = testCall.start_time - c.end_time
            if (timeSep < minTime):
                minTime = timeSep
        weighting = (sessionSeparationTime - minTime) / sessionSeparationTime
        return weighting

    # Only call this when a new call has finished
    def add_call_to_session(self, newCall):
        newCall.session = self
        self.calls.append(newCall)
        self.end_time = newCall.end_time


def find_active_session(testCall):
    likelySession = None
    maxWeighting = 0
    for s in sessionsList:
        if not s.isClosed:
            testWeight = s.isCallInThisSession(testCall)
            if (testWeight > maxWeighting):
                likelySession = s
                maxWeighting = testWeight
    # If no active session found for the radio on this call, create new and rtn
    if (maxWeighting <= 0):
        return Session(testCall)
    else:
        return likelySession


def find_active_call(testRadio, testMarker, testTime):
    for c in callsList:
        if (c.isFrameInThisCall(testRadio, testMarker)):
            return c
    # if there's no active call found, create a new call and return it
    return Call(testRadio, testMarker, testTime)


# closes old calls that haven't had updates for a while.
def cleanupObjects():
    while 1:
        time.sleep(5)
        print("===== Cleanup Thread Running =====")
        for c in callsList:
            if (lastFrameTime - c.end_time > callSeparationTime):
                c.end_call()
                print("call ended for Call from Radio: " + str(c.radio))
            if (c.isClosed):
                callsCleanupList.append(c)
        for s in sessionsList:
            if (lastFrameTime - s.end_time > sessionSeparationTime):
                s.end_session()
            if (s.isClosed):
                sessionsCleanupList.append(s)


cleanup_thread = Thread(target=cleanupObjects)
cleanup_thread.start()


def doCallsOverlap(call, testCall):
    if (call.end_time > testCall.start_time):
        if (testCall.start_time > call.start_time):
            print("DEBUG: calls overlapped")
            return True
    elif (call.end_time > testCall.end_time):
        if (testCall.end_time > call.start_time):
            print("DEBUG: calls overlapped")
            return True


time.sleep(20)
# G.add_nodes_from(callsList)
