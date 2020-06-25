CC = g++
CFLAGS = -O2 -std=c++11 -Wall -Wextra
LDFLAGS = -lz

SRC = 	decoder_main.cc coding.cc report.cc utils.cc viterbi.cc  base64.cc \
	tetra_dl.cc mac.cc llc.cc mle.cc cmce.cc cmce_sds.cc cmce_sds_lip.cc uplane.cc \
	mac_defrag.cc

OBJ = $(SRC:.cc=.o)
EXE = decoder

.cc.o:
	$(CC) $(CFLAGS) -c $< -o $@

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

test: test.o
	$(CC) $(CFLAGS) test.o -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(EXE) *.o *~

