########################################################################
#
#	FILENAME		:	makefile
#
#	DESCRIPTION		:	Usable for the compilation of the C code
#					(UNIX environment)
#
#	LANGUAGE		:	ANSI-C
#
########################################################################
#
#	COMMENTS		:	Command line to be used (without the brackets)
#
#					- "make scoder"	: to compile the source coder
#					- "make ccoder"	: to compile the channel coder
#					- "make sdecoder"	: to compile the source decoder
#					- "make cdecoder"	: to compile the channel decoder
#					- "make"		: to compile all 
#
########################################################################

# macro definitions

CC=gcc

CFLAGS	=	-I. -Wall -g -O3

SRCS1		=	scoder.c		scod_tet.c		sub_sc_d.c \
			sub_dsp.c		fbas_tet.c		fexp_tet.c \
			fmat_tet.c		tetra_op.c

SRCS2		=	ccoder.c		ccod_tet.c		sub_cc.c \
			tetra_op.c

SRCS3		=	sdecoder.c		sdec_tet.c		sub_sc_d.c \
			sub_dsp.c		fbas_tet.c		fexp_tet.c \
			fmat_tet.c		tetra_op.c

SRCS4		=	cdecoder.c		cdec_tet.c		sub_cd.c \
			tetra_op.c

# target definitions

all:		scoder	ccoder	sdecoder	cdecoder

scoder: $(SRCS1)
	$(CC) $(SRCS1) $(CFLAGS) -o scoder

ccoder: $(SRCS2)
	$(CC) $(SRCS2) $(CFLAGS) -o ccoder

sdecoder: $(SRCS3)
	$(CC) $(SRCS3) $(CFLAGS) -o sdecoder

cdecoder: $(SRCS4)
	$(CC) $(SRCS4) $(CFLAGS) -o cdecoder

clean:
	rm -f core *.o cdecoder sdecoder ccoder scoder

