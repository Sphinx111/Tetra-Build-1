#include "tetra_dl.h"
#include <signal.h>

static const int UDP_PORT_RX = 42000;                                           ///< UDP RX port (ie. where to receive bits from PHY layer)
static const int UDP_PORT_TX = 42100;                                           ///< UDP TX port (ie. where to send Json data)

/**
 * @brief Decoder program entry point
 *
 * Reads demodulated values from UDP port 42000 coming from physical demodulator
 * Writes decoded frames to UDP port 42100 to tetra interpreter
 *
 */

int main()
{
    tetra_dl *tetra_tetra_dl = new tetra_dl();        

    // prepare output socket
    struct sockaddr_in addr_output;
    memset(&addr_output, 0, sizeof(struct sockaddr_in));
    addr_output.sin_family = AF_INET;
    addr_output.sin_port = htons(UDP_PORT_TX);
    inet_aton("127.0.0.1", &addr_output.sin_addr);
   
    tetra_tetra_dl->socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    connect(tetra_tetra_dl->socketfd, (struct sockaddr *) & addr_output, sizeof(struct sockaddr));    
    printf("Output fd socket = 0x%04x\n", tetra_tetra_dl->socketfd);
    if (tetra_tetra_dl->socketfd < 0)
    {
        perror("open");
        exit(2);
    }
    
    //int fd = open("out.bits", O_RDONLY); // DEBUG

    // prepare input socket
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(UDP_PORT_RX);
    inet_aton("127.0.0.1", &addr.sin_addr);

    int sockfd_input = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    bind(sockfd_input, (struct sockaddr *)&addr, sizeof(struct sockaddr));
    printf("Input  fd socket = 0x%04x\n", sockfd_input);        
    if (sockfd_input < 0)
    {
        perror("open");
        exit(2);
    }

    uint8_t rx_buf[1024];                                                       // receive buffer
    while (1)
    {
        //int bytes_read = read(fd, rx_buf, sizeof(rx_buf)); // DEBUG
        int bytes_read = read(sockfd_input, rx_buf, sizeof(rx_buf));
        
        if (bytes_read < 0)
        {
            perror("read error");
            exit(1);
        }
        else if (bytes_read == 0)
        {
            break;
        }
        
        for (int cnt = 0; cnt < bytes_read; cnt++)
        {
            tetra_tetra_dl->rx_symbol(rx_buf[cnt]);                             // bytes must be pushed one at a time in decoder
        }
    }

    close(sockfd_input);
    close(tetra_tetra_dl->socketfd);

    //close(fd); // DEBUG

    return 0;
}
