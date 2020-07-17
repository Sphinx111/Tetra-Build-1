CC = g++
CFLAGS = -O2 -std=c++11 -Wall -Wextra
LDFLAGS = -lncurses -lz

SRC = recorder_main.cc window.cc base64.cc json_parser.cc cid.cc call_identifier.cc utils.cc
OBJ = $(SRC:.cc=.o)
EXE = recorder

.cc.o:
	$(CC) $(CFLAGS) -c $< -o $@

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

clean:
	rm -f $(EXE) *.o *~

