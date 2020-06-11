CC = g++
CFLAGS = -O2 -fPIC -std=c++11 -Wall
LDFLAGS = -ljson-c -lncurses -lz

SRC = call_identifier.cc cid.cc recorder_main.cc window.cc base64.cc
OBJ = $(SRC:.cc=.o)
EXE = recorder

.cc.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

clean:
	rm -f $(EXE) *.o *~

