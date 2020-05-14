# Tetra-kit

TETRA downlink decoder/recorder kit

# Generalities

This project aim is to provide an extensible TETRA downlink receiver kit for RTL-SDR dongles.

It tries to be as close as possible to the TETRA specification layers *ETSI EN 300 392-2 v3.4.1
(2010-08)* and transmits downlink informations in Json plain text format to be recorded or analyzed
by an external program.

The decoder is written in C++ and implements a soft synchronizer allowing missing frames (50 bursts)
before loosing synchronization.

# Workflow

The decoder get physical layer bits from gnuradio PI/4 DQPSK receiver and transmit TETRA downlink
informations in Json format to be analyzed and recorded.

Speech frames are compressed with zlib and coded in Base64 to be transmitted in Json text.

The 3 parts are linked with UDP sockets:

    Physical TX on UDP port 42000 -> receiver TX on UDP port 42100 -> recorder

# Physical layer

The physical PI/4 DQPSK gnuradio receiver is inspired of [Tim's tetra-toolkit](https://github.com/Tim---/tetra-toolkit).

It works fine with RTL-SDR dongles at 2Mbps. Results are much better than with HackRF which is more noisy.

# Decoder

The decoder role is to interpret and reconstruct TETRA packets and transmit it in Json format
for recording and analysis. Only a few fields are transmitted for now, but using Json, it can
be extensed very easily.

It implements partially the downlink MAC, LLC, MLE, CMCE and UPLANE layers.

# Recorder

The recorder interprets received Json text frames and handles the informations from Decoder.

    Everything is based on the Caller ID (CID).

It associates and maintain a database of CID, SSI and Usage markers and record the
Json speech frames onto seperate files based on those informations in the out/ folder.

Those files are then processed with TETRA codec to recover speech (unencrypted only).

# Previous work

This kit has been done thanks to the work of:
* [osmo-tetra](https://git.osmocom.org/osmo-tetra/)
* [sq5bpf osmo-tetra](https://github.com/sq5bpf/osmo-tetra-sq5bpf)
* [sq5bpf telive](https://github.com/sq5bpf/telive)
* [Min Xu Viterbi codec](https://github.com/xukmin/viterbi)

# To be done

* Defragmentation is not implemented yet
* No FEC correction
