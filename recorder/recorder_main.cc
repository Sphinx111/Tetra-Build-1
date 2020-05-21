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
#include <cstring>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <json-c/json.h>
#include <json-c/json_util.h>
#include "base64.h"
#include <zlib.h>

#include "recorder.h"
#include "call_identifier.h"
#include "window.h"

/*
 * Simple Tetra recorder with ncurses ui
 *
 * Read informations and speech frames from decoder on UDP socket
 *
 * HISTORY:
 *   2016-09-11  LT  0.0  First release
 *   2020-05-08  LT  0.1  Updated for Json input
 *
 * NOTES:
 *    ssi              Short Suscriber Identity, identify all devices
 *    call_identifier  Call identifier, all communication are based on this
 *    usage_marker     (see 21.4.3.1 MAC-RESOURCE PDU address = SSI 24 bits + usage marker 6 bits (2^6=64)
 *                     possible values in usage marker 21.4.7 -> reserved 000000, 000001, 000010, 000011
 *
 */

static vector<call_identifier_t> cid_list;                                      ///< call identifiers list

/**
 * @brief API to access to CID list from other modules
 *
 */

call_identifier_t * get_cid(int index)
{
    call_identifier_t * res = NULL;

    if (cid_list.size() > 0)
    {
        if (index < (int)cid_list.size())
        {
            res = &cid_list[index];
        }
    }

    return res;
}

static const int TIME_WAIT_MS = 50;                                             // udp port maximum waiting time [ms]
static const int UDP_PORT     = 42100;                                          // rx UDP port

static int fd_sock_rx;                                                          // rx socket handle

/**
 * @brief Returns true if CID exists is in list
 *
 */

static bool cid_exists(uint32_t cid)
{
    bool b_exists = false;

    for (size_t cnt = 0; cnt < cid_list.size(); cnt++)
    {
        if (cid_list[cnt].m_cid == cid)
        {
            b_exists = true;
            break;
        }
    }

    return b_exists;
}

/**
 * @brief Return CID index in list or ((size_t)-1) if not.
 *        This last value shouldn't be used since we have to check
 *        if the CID exists before
 *
 */

static size_t cid_index(uint32_t cid)
{
    size_t index = -1;

    for (size_t cnt = 0; cnt < cid_list.size(); cnt++)
    {
        if (cid_list[cnt].m_cid == cid)
        {
            index = cnt;
            break;
        }
    }

    return index;
}

/**
 * @brief Return CID index in list for a given usage marker or ((size_t)-1) if there is none associated.
 *
 */

static bool cid_index_by_usage_marker(uint8_t usage_marker, size_t * index)
{
    bool ret = false;

    for (size_t cnt = 0; cnt < cid_list.size(); cnt++)
    {
        if (cid_list[cnt].m_usage_marker == usage_marker)
        {
            *index = cnt;
            ret = true;
            break;
        }
    }

    return ret;
}

/**
 * @brief Add CID in list if not already there
 *
 */

static void cid_add(uint32_t cid)
{
    if (!cid_exists(cid))
    {
        cid_list.push_back(call_identifier_t(cid));
    }
}

/**
 * @brief Associate a SSI to a given CID. Add the CID is it doesn't exists.
 *        Update last seen SSI time.
 *
 */

static void add_ssi_to_cid(uint32_t cid, uint32_t ssi)
{
    if (ssi <= 0) return;

    if (!cid_exists(cid))
    {
        cid_add(cid);
    }

    size_t index = cid_index(cid);

    // check if ssi already exists in cid list
    bool b_exists = false;

    for (size_t cnt = 0; cnt < cid_list[index].m_ssi.size(); cnt++)
    {
        if (cid_list[index].m_ssi[cnt].ssi == ssi)
        {
            time(&cid_list[index].m_ssi[cnt].last_seen);                        // SSI exists, update its last seen time
            b_exists = true;
            break;
        }
    }

    if (!b_exists)                                                              // if not exists, add new ssi to list
    {
        ssi_t new_ssi;
        new_ssi.ssi = ssi;
        time(&new_ssi.last_seen);
        cid_list[index].m_ssi.push_back(new_ssi);
    }
}

/**
 * @brief Update CID usage marker
 *
 * TODO check CID value to ensure CID is a valid number
 *
 */

static void cid_update_usage_marker(uint32_t cid, uint8_t usage_marker)
{
    if (!cid_exists(cid))                                                       // if cid doesn't exists, create it
    {
        cid_add(cid);
    }

    size_t index = cid_index(cid);
    cid_list[index].update_usage_marker(usage_marker);
}

/**
 * @brief Release a given CID from list
 *
 */

static void cid_release(uint32_t cid)
{
    if (!cid_exists(cid)) return;

    for (vector<call_identifier_t>::iterator it = cid_list.begin(); it != cid_list.end();)
    {
        if ((*it).m_cid == cid)
        {
            it = cid_list.erase(it);                                            // erase current id and get pointer to next one
        }
        else
        {
            ++it;                                                               // go to next element
        }
    }
}

/**
 * @brief Clean up CID/SSI with their last seen time
 *
 * TODO to finish and test in class_itentifier_t
 *
 */

static void cid_clean_up()
{
    time_t now;
    time(&now);

    for (size_t idx = 0; idx < cid_list.size(); idx++)
    {
        cid_list[idx].clean_up();
    }
}

/**
 * @brief Send traffic speech frame to a CID identified by a given usage marker
 *
 */

static void send_traffic_to_cid_by_usage_marker(uint8_t usage_marker, const char * data, uint32_t len)
{
    if (usage_marker > 63) return;                                              // only values from 0-63 are relevant for TETRA

    size_t index;

    if (cid_index_by_usage_marker(usage_marker, &index))
    {
        cid_list[index].push_traffic(data, len);                                // push traffic to this cid
    }
}

#if 0
static void push_traffic(const char * data, uint32_t len, uint8_t usage_marker)
{
    char buf[2048];
    sprintf(buf, "out/record_%u.out", usage_marker);
    FILE * file = fopen(buf, "ab");
    fwrite(data, 1, len, file);
    fflush(file);
    fclose(file);
}
#endif

/**
 * @brief Main function which process Json traffic and associate SSI, CID, etc...
 *
 * WARNING: D-INFO PDU mustn't be used to allocate a call id (see p. 136)
 *
 */

static void parse_pdu(const char * data, FILE * fd_log)
{
    struct json_object * jobj = json_tokener_parse(data);                       // parse informations from buffer

    if (jobj == NULL) return;                                                   // can't parse Json

    struct json_object * tmp;

    // extract data common to all pdu
    json_object_object_get_ex(jobj, "service", &tmp);
    string service = json_object_get_string(tmp);

    json_object_object_get_ex(jobj, "pdu", &tmp);
    string pdu = json_object_get_string(tmp);

    json_object_object_get_ex(jobj, "usage marker", &tmp);
    uint8_t usage_marker = (uint8_t)json_object_get_int(tmp);

    json_object_object_get_ex(jobj, "ssi", &tmp);
    uint32_t ssi = (uint32_t)json_object_get_int(tmp);

    bool b_log = true;

    if (!service.compare("MAC"))                                                // MAC service (don't print)
    {
        b_log = false;                                                          // too much packets to log
    }
    else if (!service.compare("UPLANE"))                                        // traffic speech frame
    {

        json_object_object_get_ex(jobj, "uzsize", &tmp);                        // uncompressed frame length 2 * 690 + 1 bytes
        uint64_t zlib_uncomp_size = json_object_get_int(tmp);

        json_object_object_get_ex(jobj, "zsize", &tmp);                         // compressed frame length (before B64 since B64 add overhead)
        uint64_t zlib_comp_size = json_object_get_int(tmp);

        json_object_object_get_ex(jobj, "frame", &tmp);                         // zlib + B64 frame
        string frame = json_object_get_string(tmp);

        const int BUFSIZE = 4096;

        // base64 decode
        unsigned char buf_b64out[BUFSIZE] = {0};
        //uint32_t len_out =
        b64_decode((const unsigned char *)frame.c_str(), frame.length(), buf_b64out);

        // zlib uncompress
        char buf_zlib_out[BUFSIZE] = {0};
        int ret = uncompress((Bytef *)buf_zlib_out, &zlib_uncomp_size, (Bytef *)buf_b64out, zlib_comp_size);

        if (!ret) send_traffic_to_cid_by_usage_marker(usage_marker, buf_zlib_out, zlib_uncomp_size); // process it
    }
    else                                                                        // other services
    {
        if ((!pdu.compare("D-ALERT")) ||
            (!pdu.compare("D-CONNECT")) ||
            (!pdu.compare("D-CONNECT ACK")) ||
            (!pdu.compare("D-SETUP")) ||
            (!pdu.compare("D-STATUS")) ||
            (!pdu.compare("D-TX GRANTED")) ||
            (!pdu.compare("D-TX CEASED")))
        {
            // register new cid and attach ssi and usage marker
            json_object_object_get_ex(jobj, "call identifier", &tmp);
            uint32_t cid = (uint32_t)json_object_get_int(tmp);

            cid_update_usage_marker(cid, usage_marker);                         // CID will be added to list if it doesn't exists yet
            add_ssi_to_cid(cid, ssi);
            scr_update(data);
        }
        else if (!pdu.compare("D-RELEASE"))                                     // || (!pdu.compare("D-TX WAIT")))
        {
            // release cid
            json_object_object_get_ex(jobj, "call identifier", &tmp);
            uint32_t cid = (uint32_t)json_object_get_int(tmp);

            cid_release(cid);
            scr_update(data);
        }
        else if ((!pdu.compare("D-SDS-DATA")) ||
                 (!pdu.compare("D-STATUS")))                                    // SDS messages
        {
            scr_print_middle(data);                                             // note that there is two lines printed for every message (for analyze, we provide full hexa + decoded message)
        }
        else
        {

        }

        //cid_clean_up();                                                         // call garbage collector
    }

    if (b_log)                                                                  // append to log file
    {
        struct {
            int flag;
            const char *flag_str;
        } json_flags = { JSON_C_TO_STRING_NOZERO, "JSON_C_TO_STRING_NOZERO" };  // remove all empty spaces between fields

        fprintf(fd_log, "%s\n", json_object_to_json_string_ext(jobj, json_flags.flag));
        fflush(fd_log);
    }

    json_object_put(tmp);                                                       // clean objects
    json_object_put(jobj);
}

/**
 * @brief Initialize all required stuff
 *
 */

void init()
{
    // initialize screen and data
    scr_init();

    // other data
    cid_list.clear();
}

/**
 * @brief Receive from UDP socket with time-out
 *
 */

int timed_recv(char * msg, size_t max_size, int max_wait_ms)
{
    struct sockaddr_in si_other;
    socklen_t socket_len = sizeof(si_other);

    fd_set fdset;
    FD_ZERO(&fdset);                                                            // clear fd set
    FD_SET(fd_sock_rx, &fdset);                                                 // add socket fd to set

    struct timeval timeout;
    timeout.tv_sec  =  max_wait_ms / 1000;
    timeout.tv_usec = (max_wait_ms % 1000) * 1000;

    int ret = select(fd_sock_rx + 1, &fdset, &fdset, &fdset, &timeout);         // monitor socket for max_wait_ms
    int val = -1;

    if (ret > 0)                                                                // socket has data, then receive it
    {
        val = recvfrom(fd_sock_rx, msg, max_size, 0, (struct sockaddr *)&si_other, &socket_len);
    }
    else                                                                        // invalid or no data
    {
        val = -1;
    }

    return val;
}

/**
 * @brief Debug function to replay from a log.txt like's text file (Json)
 *
 */

void replay_from_input_log_file(const char * file_name)
{
    const int BUFLEN = 8192;

    char buf[BUFLEN];

    FILE * fd_in = fopen(file_name, "rt");
    FILE * fd_log = fopen("replayed.txt", "wt");

    while (fgets(buf, sizeof(buf), fd_in))
    {
        parse_pdu(buf, fd_log);
        printf("%s", buf);
    }

    fclose(fd_in);
    fclose(fd_log);
}

/**
 * @brief Program entry point
 *
 */

int main()
{
    init();

    /*replay_from_input_log_file("input.txt"); // DEBUG read input.txt (copy your log.txt file to input.txt so it is not overwritten)
    scr_clean();
    exit(0);*/

    // listening RX socket
    struct sockaddr_in sockfd_in;
    memset(&sockfd_in, 0, sizeof(sockfd_in));                                   // prepare socket for listening
    sockfd_in.sin_family      = AF_INET;
    sockfd_in.sin_port        = htons(UDP_PORT);
    sockfd_in.sin_addr.s_addr = htonl(INADDR_ANY);

    fd_sock_rx = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (fd_sock_rx == -1)
    {
        perror("socket error");
    }

    if (bind(fd_sock_rx, (struct sockaddr *)&sockfd_in, sizeof(sockfd_in)) == -1)
    {
        perror("bind error");
    }

    // log file
    FILE * fd_log = fopen("log.txt", "at");

    const int BUFLEN = 8192;
    char buf_rx[BUFLEN];

    while (1)
    {
        int len = timed_recv(buf_rx, BUFLEN, TIME_WAIT_MS);

        if (len > 0)
        {
            parse_pdu(buf_rx, fd_log);
        }

    }

    fclose(fd_log);
    close(fd_sock_rx);

    scr_clean();

    return 0;
}
