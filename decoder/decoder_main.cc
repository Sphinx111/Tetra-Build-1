/*
 *  tetra-kit
 *  Copyright (C) 2020  LarryTh <dev@logami.fr>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "tetra_dl.h"
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

/** @brief Program working mode enumeration */

enum program_mode_t {
    STANDARD_MODE         = 0,
    READ_FROM_BINARY_FILE = 1,
    SAVE_TO_BINARY_FILE   = 2,
};

/** @brief interrupt flag */

static volatile int sigint_flag = 0;

/**
 * @brief handle SIGINT to clean up
 *
 */

static void sigint_handler(int val)
{
    sigint_flag = 1;
}

/**
 * @brief Decoder program entry point
 *
 * Reads demodulated values from UDP port 42000 coming from physical demodulator
 * Writes decoded frames to UDP port 42100 to tetra interpreter
 *
 * Filtering log for SDS: sed -n '/SDS/ p' log.txt > out.txt
 *
 */

int main(int argc, char * argv[])
{
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, 0);

    int udp_port_rx = 42000;                                                    // UDP RX port (ie. where to receive bits from PHY layer)
    int udp_port_tx = 42100;                                                    // UDP TX port (ie. where to send Json data)

    const int FILENAME_LEN = 256;
    char opt_filename_in[FILENAME_LEN]  = "";                                   // input bits filename
    char opt_filename_out[FILENAME_LEN] = "";                                   // output bits filename

    int program_mode = STANDARD_MODE;
    int debug_level = 0;
    bool fill_bit_flag = true;

    int option;
    while ((option = getopt(argc, argv, ":hr:t:i:o:d:f")) != -1)
    {
        switch (option)
        {
        case 'r':
            udp_port_rx = atoi(optarg);
            break;

        case 't':
            udp_port_tx = atoi(optarg);
            break;

        case 'i':
            strncpy(opt_filename_in, optarg, FILENAME_LEN - 1);
            program_mode |= READ_FROM_BINARY_FILE;
            break;

        case 'o':
            strncpy(opt_filename_out, optarg, FILENAME_LEN - 1);
            program_mode |= SAVE_TO_BINARY_FILE;
            break;

        case 'd':
            debug_level = atoi(optarg);
            break;

        case 'f':
            fill_bit_flag = false;
            break;

        case 'h':
            printf("\nUsage: ./decoder [OPTIONS]\n\n"
                   "Options:\n"
                   "  -r <UDP socket> receiving from phy [default port is 42000]\n"
                   "  -t <UDP socket> sending Json data [default port is 42100]\n"
                   "  -i <file> replay data from binary file instead of UDP\n"
                   "  -o <file> record data to binary file (can be replayed with -i option)\n"
                   "  -d <level> print debug information\n"
                   "  -f keep fill bits\n"
                   "  -h print this help\n\n");
            exit(EXIT_FAILURE);
            break;

        case '?':
            printf("unkown option, run ./decoder -h to list available options\n");
            exit(EXIT_FAILURE);
            break;
        }
    }

    // create decoder

    tetra_dl * decoder = new tetra_dl(debug_level, fill_bit_flag);

    // output destination socket

    struct sockaddr_in addr_output;
    memset(&addr_output, 0, sizeof(struct sockaddr_in));
    addr_output.sin_family = AF_INET;
    addr_output.sin_port = htons(udp_port_tx);
    inet_aton("127.0.0.1", &addr_output.sin_addr);

    decoder->socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);               // assign and connect UDP tx socket to decoder
    connect(decoder->socketfd, (struct sockaddr *) & addr_output, sizeof(struct sockaddr));

    printf("Output socket 0x%04x on port %d\n", decoder->socketfd, udp_port_tx);

    if (decoder->socketfd < 0)
    {
        perror("Couldn't create output socket");
        exit(EXIT_FAILURE);
    }

    // output file if any

    int fd_save = 0;

    if (program_mode & SAVE_TO_BINARY_FILE)                                     // save input bits to file
    {
        fd_save = open(opt_filename_out, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP);
        if (fd_save < 0)
        {
            fprintf(stderr, "Couldn't open output file");
            exit(EXIT_FAILURE);
        }
    }

    // input source

    int fd_input = 0;

    if (program_mode & READ_FROM_BINARY_FILE)                                   // read input bits from file
    {
        fd_input = open(opt_filename_in, O_RDONLY);

        printf("Input from file '%s' 0x%04x\n", opt_filename_in, fd_input);

        if (fd_input < 0)
        {
            fprintf(stderr, "Couldn't open input bits file");
            exit(EXIT_FAILURE);
        }
    }
    else                                                                        // read input bits from UDP socket
    {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(udp_port_rx);
        inet_aton("127.0.0.1", &addr.sin_addr);

        fd_input = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        bind(fd_input, (struct sockaddr *)&addr, sizeof(struct sockaddr));

        printf("Input socket 0x%04x on port %d\n", fd_input, udp_port_rx);

        if (fd_input < 0)
        {
            fprintf(stderr, "Couldn't create input socket");
            exit(EXIT_FAILURE);
        }
    }

    const int RXBUF_LEN = 1024;
    uint8_t rx_buf[RXBUF_LEN];                                                  // receive buffer

    while (!sigint_flag)
    {
        int bytes_read = read(fd_input, rx_buf, sizeof(rx_buf));

        if (errno == EINTR)
        {
            fprintf(stderr, "EINTR\n");                                         // print is required for ^C to be handled
            break;
        }
        else if (bytes_read < 0)
        {
            fprintf(stderr, "Read error\n");
            break;
        }
        else if (bytes_read == 0)
        {
            break;
        }

        if (program_mode & SAVE_TO_BINARY_FILE)
        {
            write(fd_save, rx_buf, bytes_read);
        }

        for (int cnt = 0; cnt < bytes_read; cnt++)
        {
            decoder->rx_symbol(rx_buf[cnt]);                                    // bytes must be pushed one at a time into decoder
        }
    }

    close(decoder->socketfd);

    close(fd_input);                                                            // file or socket must be closed

    if (program_mode & SAVE_TO_BINARY_FILE)                                     // close save file only if openede
    {
        close(fd_save);
    }

    delete decoder;

    printf("Clean exit\n");

    return EXIT_SUCCESS;
}
