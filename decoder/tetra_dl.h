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
#ifndef TETRA_DL_H
#define TETRA_DL_H
#include <cstdio>
#include <string.h>
#include <vector>
#include <sstream>

#include <unistd.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include <json-c/json.h>
#include <json-c/json_util.h>

#include "viterbi.h"

using namespace std;

/** 
 * @defgroup tetra_dl TETRA decoder
 *
 * @{
 *
 */


/*
 * TETRA decoder for pi/4-DQPSK modulation
 *
 * EN 300 392-2 - v3.4.1
 *
 * Freq: 467.5625 MHz
 *       466.6875 MHz
 *
 * NOTE:
 *  - only decode downlink
 *  - only decode continuous downlink burst channel
 *  - MAC fragmentation not handled
 *  - LLC fragmentation not handled
 *  - Viterbi codec is handling string, not optimized
 *
 */

/**
 * @brief Burst type
 *
 * 9.4.4 - type of bursts
 *
 * NOTE:
 *   - we only decode continuous downlink bursts
 *
 */

enum burst_t {
    SB     = 0,                                                                 // synchronisation downlink burst 9.4.4.3.4
    NDB    = 1,                                                                 // 1 logical channel in time slot TCH or SCH/F
    NDB_SF = 2                                                                  // 2 logical channels in time slot STCH+TCH or STCH+STCH or SCH/HD+SCH/HD or SCH/HD+BNCH 9.4.4.3.2
};

/**
 * @brief Logical channels enum
 *
 */

enum mac_logical_channel_t {                                                    // CP only
    AACH   = 0,
    BLCH   = 1,
    BNCH   = 2,
    BSCH   = 3,
    SCH_F  = 4,
    SCH_HD = 5,
    STCH   = 6,
    TCH_S  = 7,
    TCH    = 8,
    unkown = 9
};

/**
 * @brief Tetra sub-system synchronisation - ch. 7
 *
 */

struct tetra_time_t {
    uint16_t fn;                                                                ///< frame number
    uint16_t mn;                                                                ///< multi-frame number
    uint16_t tn;                                                                ///< time slot
};

/**
 * @brief Tetra cell information
 *
 */

struct tetra_cell_infos_t {
    uint16_t color_code;                                                        ///< Cell Color code
    uint32_t mcc;                                                               ///< Cell MCC 
    uint32_t mnc;                                                               ///< Cell MNC
    uint32_t scrambling_code;                                                   ///< Cell Scrambling code
};

/**
 * @brief Tetra MAC address
 *
 * Contains the current burst address state (21.4.3.1)
 *
 */

struct mac_address_t {
    uint8_t  address_type;
    uint8_t  event_label;
    uint8_t  usage_marker;
    uint8_t  stolen_flag;
    uint32_t smi;
    uint32_t ssi;
    uint32_t ussi;
};

/**
 * @brief Downlink usage values 21.4.7.2 table 21.77
 *
 */

enum downlink_usage_t {
    UNALLOCATED      = 0,
    ASSIGNED_CONTROL = 1,
    COMMON_CONTROL   = 2,
    RESERVED         = 3,
    TRAFFIC          = 4                                                        // traffic usage marker assigned in MAC-RESOURCE PDU
};

/**
 * @brief Contains the current MAC informations for routing to logical channels
 *
 */

struct mac_state_t {
    downlink_usage_t      downlink_usage;                                       ///< Downlink usage type
    uint32_t              downlink_usage_marker;                                ///< Downlink usage marker
    mac_logical_channel_t logical_channel;                                      ///< Current logical channel
};

/**
 * @brief TETRA downlink decoder class
 *
 * HISTORY:
 *   - 2018-12-08  LT  0.0  first release
 *
 */

class tetra_dl {
public:
    tetra_dl();
    ~tetra_dl();

    // burst data
    vector<uint8_t> g_frame_data;                                               ///< Burst data
    uint32_t        g_frame_len;                                                ///< Burst length in bits

    // timing and burst synchronizer
    tetra_time_t       g_time;                                                  ///< Tetra timing
    tetra_cell_infos_t g_cell_infos;                                            ///< Cell informations
    
    bool     g_cell_informations_acquired;                                      ///< Cell informations have been acquired
    bool     g_is_synchronized;                                                 ///< True is program is synchronized with burst
    uint64_t g_sync_bit_counter;                                                ///< Synchronization bits counter

    int rx_symbol(uint8_t sym);
    void process_frame();
    void print_data();
    void reset_synchronizer();
    void increment_tn();

    void calculate_scrambling_code();

    // decoding functions per clause 8
    ViterbiCodec * viterbi_codec16_14;                                          ///< Viterbi codec
    vector<uint8_t> dec_descramble(vector<uint8_t> data, int len, uint32_t ScramblingCode);
    vector<uint8_t> dec_deinterleave(vector<uint8_t> data, uint32_t K, uint32_t a);
    vector<uint8_t> dec_depuncture23(vector<uint8_t> data, uint32_t len);
    vector<uint8_t> dec_viterbi_decode16_14(vector<uint8_t> data);
    vector<uint8_t> dec_reed_muller_3014_decode(vector<uint8_t> data);

    // CRC16 check
    int check_crc16ccitt(vector<uint8_t> data, int len);

    // MAC
    mac_state_t   mac_state;                                                    ///< Current MAC state (from ACCESS-ASSIGN PDU)
    mac_address_t mac_address;                                                  ///< Current MAc address (from MAC-RESOURCE PDU)
    uint8_t       second_slot_stolen_flag;                                      ///< 1 if second slot is stolen

    void service_lower_mac(vector<uint8_t> data, int burst_type);
    void service_upper_mac(vector<uint8_t> data, mac_logical_channel_t mac_logical_channel);

    vector<uint8_t> mac_pdu_process_sync(vector<uint8_t> pdu);                  // process SYNC
    void            mac_pdu_process_aach(vector<uint8_t> data);                 // process ACCESS-ASSIGN - no SDU
    vector<uint8_t> mac_pdu_process_ressource(vector<uint8_t> pdu);             // process MAC-RESSOURCE
    vector<uint8_t> mac_pdu_process_sysinfo(vector<uint8_t> pdu);               // process SYSINFO
    vector<uint8_t> mac_pdu_process_mac_frag(vector<uint8_t> pdu);              // process MAC-FRAG
    vector<uint8_t> mac_pdu_process_mac_end(vector<uint8_t> pdu);               // process MAC-END
    vector<uint8_t> mac_pdu_process_d_block(vector<uint8_t> pdu);               // process MAC-D-BLCK

    // LLC
    void service_llc(vector<uint8_t> pdu, mac_logical_channel_t mac_logical_channel);

    // MLE
    void service_mle( vector<uint8_t> pdu, mac_logical_channel_t mac_logical_channel);
    void mle_process_d_nwrk_broadcast(vector<uint8_t> pdu);
    void mle_process_d_nwrk_broadcast_extension(vector<uint8_t> pdu);

    // CMCE
    void service_cmce(vector<uint8_t> pdu, mac_logical_channel_t mac_logical_channel);
    void cmce_parse_d_alert(vector<uint8_t> pdu);
    void cmce_parse_d_call_restore(vector<uint8_t> pdu);
    void cmce_parse_d_connect(vector<uint8_t> pdu);
    void cmce_parse_d_connect_ack(vector<uint8_t> pdu);
    void cmce_parse_d_disconnect(vector<uint8_t> pdu);
    void cmce_parse_d_release(vector<uint8_t> pdu);
    void cmce_parse_d_setup(vector<uint8_t> pdu);
    void cmce_parse_d_tx_ceased(vector<uint8_t> pdu);
    void cmce_parse_d_tx_continue(vector<uint8_t> pdu);
    void cmce_parse_d_tx_granted(vector<uint8_t> pdu);
    void cmce_parse_d_tx_interrupt(vector<uint8_t> pdu);
    void cmce_parse_d_tx_wait(vector<uint8_t> pdu);

    // CMCE SDS sub-entity
    void cmce_sds_parse_d_sds_data(vector<uint8_t> pdu);
    void cmce_sds_parse_d_status(vector<uint8_t> pdu);
    void cmce_sds_parse_type4_data(vector<uint8_t> pdu, const uint16_t len);
    void cmce_sds_parse_sub_d_transfer(vector<uint8_t> pdu, const uint16_t len);
    void cmce_sds_parse_simple_text_messaging(vector<uint8_t> pdu, const uint16_t len);
    void cmce_sds_parse_simple_location_system(vector<uint8_t> pdu, const uint16_t len);
    
    // U-plane
    void service_u_plane(vector<uint8_t> data, mac_logical_channel_t mac_logical_channel); // U-plane traffic

    // for reporting informations in Json format
    struct json_object * jobj;                                                  ///< Json object
    int socketfd;                                                               ///< UDP socket to write to

    void report_start(const string service, const string pdu);
    void report_start(const char * service, const char * pdu);
    void report_add(const char * field, const char *val);
    void report_add(string field, string val);
    void report_add(string field, uint8_t val);
    void report_add(string field, uint16_t val);
    void report_add(string field, uint32_t val);
    void report_add(string field, uint64_t val);
    void report_add(string field, vector<uint8_t> vec);
    void report_add_compressed(string field, const unsigned char * binary_data, uint16_t data_len);
    void report_send();

private:
    // 9.4.4.3.2 Normal training sequence
    const vector<uint8_t> normal_training_sequence1       = {1,1,0,1,0,0,0,0,1,1,1,0,1,0,0,1,1,1,0,1,0,0}; // n1..n22
    const vector<uint8_t> normal_training_sequence2       = {0,1,1,1,1,0,1,0,0,1,0,0,0,0,1,1,0,1,1,1,1,0}; // p1..p22
    const vector<uint8_t> normal_training_sequence3_begin = {0,0,0,1,1,0,1,0,1,1,0,1};                     // q11..q22
    const vector<uint8_t> normal_training_sequence3_end   = {1,0,1,1,0,1,1,1,0,0};                         // q1..q10

    // 9.4.4.3.4 Synchronisation training sequence
    const vector<uint8_t> synchronization_training_sequence = {1,1,0,0,0,0,0,1,1,0,0,1,1,1,0,0,1,1,1,0,1,0,0,1,1,1,0,0,0,0,0,1,1,0,0,1,1,1}; // y1..y38
};

/** @} */

#endif /* TETRA_DL_H */
