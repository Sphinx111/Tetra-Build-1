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
#include "utils.h"

static int cur_burst_type;

/**
 * @brief Returns MAC logical channel name
 *
 */

std::string tetra_dl::mac_logical_channel_name(int val)
{
    std::string ret = "";

    switch (val)
    {
    case AACH:
        ret = "AACH";
        break;

    case BLCH:
        ret = "BLCH";
        break;

    case BNCH:
        ret = "BNCH";
        break;

    case BSCH:
        ret = "BSCH";
        break;

    case SCH_F:
        ret = "SCH_F";
        break;

    case SCH_HD:
        ret = "SCH_HD";
        break;

    case STCH:
        ret = "STCH";
        break;

    case TCH_S:
        ret = "TCH_S";
        break;

    case unkown:
        ret = "unknown";
        break;

    default:
        ret = "out of range";
        break;
    }

    return ret;
}

/**
 * @brief Returns PHY burst name
 *
 */

std::string tetra_dl::burst_name(int val)
{
    std::string ret = "";

    switch (val)
    {
    case SB:
        ret = "SB";
        break;

    case NDB:
        ret = "NDB";
        break;

    case NDB_SF:
        ret = "NDB_SF";
        break;

    default:
        break;
    }

    return ret;
}

/**
 * @brief Lower MAC entry point
 *
 * Mapping of logical channels see 9.5.1 CP, TP and 9.5.1b UP
 *
 * MAC can be in "signalling and packet" (signalling mode) or "circuit mode" (traffic mode)
 *
 * Downlink logical channels:
 *    AACH on every burst
 *    BNCH mapped on bkn1 when FN==18 and (MN+TN) % 4 = 1
 *    BSCH mapped on bkn2 when FN==18 and (MN+TN) % 4 = 3
 *    SCH
 *    TCH
 *    STCH
 *
 * Notes:
 *   - AACH must be processed first to get traffic or signalling mode
 *   - Fill bit deletion to be tested (see 23.4.3.2)
 *   - TODO PDU dissociation (see 23.4.3.3)
 *
 */

void tetra_dl::service_lower_mac(std::vector<uint8_t> data, int burst_type)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - burst = %s data = %s\n", "service_lower_mac", burst_name(burst_type).c_str(), vector_to_string(data, data.size()).c_str());
        fflush(stdout);
    }

    bool bnch_flag = false;
    //bool bsch_flag = false;

    cur_burst_type = burst_type;

    if (g_time.fn==18)
    {
        if ((g_time.mn + g_time.tn) % 4 == 1)                                   // on NDB
        {
            bnch_flag = true;
        }
        else if ((g_time.mn + g_time.tn) % 4 == 3)                              // on SB
        {
            //bsch_flag = true;
        }
    }

    //printf("BURST %d\n", burst_type);
    second_slot_stolen_flag = 0;                                                // stolen flag lifetime is NDB_SF burst life only

    std::vector<uint8_t> bkn1;                                                  // busrt block BKN1
    std::vector<uint8_t> bkn2;                                                  // burst block BKN2
    std::vector<uint8_t> bbk;                                                   // burst block BBK

    if (burst_type == SB)                                                       // synchronisation burst
    {
        // BKN1 block - BSCH - SB seems to be sent only on FN=18 thus BKN1 contains only BSCH
        bkn1 = vector_extract  (data, 94,  120);
        bkn1 = dec_descramble  (bkn1, 120, 0x0003);                             // descramble with predifined code 0x0003
        bkn1 = dec_deinterleave(bkn1, 120, 11);                                 // deinterleave 120, 11
        bkn1 = dec_depuncture23(bkn1, 120);                                     // depuncture with 2/3 rate 120 bits -> 4 * 80 bits before Viterbi decoding
        bkn1 = dec_viterbi_decode16_14(bkn1);                                   // Viterbi decode - see 8.3.1.2  (K1 + 16, K1) block code with K1 = 60
        if (check_crc16ccitt(bkn1, 76))                                         // BSCH found process immediately to calculate scrambling code
        {
            service_upper_mac(bkn1, BSCH);                                      // only 60 bits are meaningful
        }

        // BBK block - AACH
        bbk = vector_extract  (data, 252, 30);                                  // BBK
        bbk = dec_descramble  (bbk,  30, g_cell_infos.scrambling_code);         // descramble
        bbk = dec_reed_muller_3014_decode(bbk);                                 // Reed-Muller correction
        service_upper_mac(bbk, AACH);

        // BKN2 block
        bkn2 = vector_extract  (data, 282, 216);
        bkn2 = dec_descramble  (bkn2, 216, g_cell_infos.scrambling_code);       // descramble
        bkn2 = dec_deinterleave(bkn2, 216, 101);                                // deinterleave
        bkn2 = dec_depuncture23(bkn2, 216);                                     // depuncture with 2/3 rate 144 bits -> 4 * 144 bits before Viterbi decoding
        bkn2 = dec_viterbi_decode16_14(bkn2);                                   // Viterbi decode
        if (check_crc16ccitt(bkn2, 140))                                        // check CRC
        {
            bkn2 = vector_extract(bkn2, 0, 124);
            service_upper_mac(bkn2, SCH_HD);
        }
    }
    else if (burst_type == NDB)                                                 // 1 logical channel in time slot
    {
        // BBK block
        bbk = vector_append(vector_extract(data, 230, 14), vector_extract(data, 266, 16)); // BBK is in two parts
        bbk = dec_descramble(bbk, 30, g_cell_infos.scrambling_code);                       // descramble
        bbk = dec_reed_muller_3014_decode(bbk);                                            // Reed-Muller correction
        service_upper_mac(bbk, AACH);

        // BKN1 + BKN2
        bkn1 = vector_append(vector_extract(data, 14, 216), vector_extract(data, 282, 216)); // reconstruct block to BKN1
        bkn1 = dec_descramble(bkn1, 432, g_cell_infos.scrambling_code);                      // descramble

        if ((mac_state.downlink_usage == TRAFFIC) && (g_time.fn <= 17))         // traffic mode
        {
            service_upper_mac(bkn1, TCH_S);                                     // frame is sent directly to User plane
        }
        else                                                                    // signalling mode
        {
            bkn1 = dec_deinterleave(bkn1, 432, 103);                            // deinterleave
            bkn1 = dec_depuncture23(bkn1, 432);                                 // depuncture with 2/3 rate 288 bits -> 4 * 288 bits before Viterbi decoding
            bkn1 = dec_viterbi_decode16_14(bkn1);                               // Viterbi decode
            if (check_crc16ccitt(bkn1, 284))                                    // check CRC
            {
                bkn1 = vector_extract(bkn1, 0, 268);
                service_upper_mac(bkn1, SCH_F);
            }
        }
    }
    else if (burst_type == NDB_SF)                                              // NDB with stolen flag
    {
        bool bkn1_valid = false;
        bool bkn2_valid = false;

        // BBK block - AACH
        bbk = vector_append(vector_extract(data, 230, 14), vector_extract(data, 266, 16)); // BBK is in two parts
        bbk = dec_descramble(bbk, 30, g_cell_infos.scrambling_code);                       // descramble
        bbk = dec_reed_muller_3014_decode(bbk);                                            // Reed-Muller correction
        service_upper_mac(bbk, AACH);

        // BKN1 block - always SCH/HD (CP channel)
        bkn1 = vector_extract  (data, 14, 216);
        bkn1 = dec_descramble  (bkn1, 216, g_cell_infos.scrambling_code);       // descramble
        bkn1 = dec_deinterleave(bkn1, 216, 101);                                // deinterleave
        bkn1 = dec_depuncture23(bkn1, 216);                                     // depuncture with 2/3 rate 144 bits -> 4 * 144 bits before Viterbi decoding
        bkn1 = dec_viterbi_decode16_14(bkn1);                                   // Viterbi decode
        if (check_crc16ccitt(bkn1, 140))                                        // check CRC
        {
            bkn1 = vector_extract(bkn1, 0, 124);
            bkn1_valid = true;
        }

        // BKN2 block - SCH/HD or BNCH
        bkn2 = vector_extract  (data, 282, 216);
        bkn2 = dec_descramble  (bkn2, 216, g_cell_infos.scrambling_code);       // descramble
        bkn2 = dec_deinterleave(bkn2, 216, 101);                                // deinterleave
        bkn2 = dec_depuncture23(bkn2, 216);                                     // depuncture with 2/3 rate 144 bits -> 4 * 144 bits before Viterbi decoding
        bkn2 = dec_viterbi_decode16_14(bkn2);                                   // Viterbi decode
        if (check_crc16ccitt(bkn2, 140))                                        // check CRC
        {
            bkn2 = vector_extract(bkn2, 0, 124);
            bkn2_valid = true;
        }

        if ((mac_state.downlink_usage == TRAFFIC) && (g_time.fn <= 17))         // traffic mode
        {
            if (bkn1_valid)
            {
                service_upper_mac(bkn1, STCH);                                  // first block is stolen for C or U signalling
            }

            if (second_slot_stolen_flag)                                        // if second slot is also stolen
            {
                if (bkn2_valid)
                {
                    service_upper_mac(bkn2, STCH);                              // second block also stolen, reset flag
                }
            }
            else                                                                // second slot not stolen, so it is still traffic mode
            {
                // do nothing, TCH/4.2 and 2.8 not taken into account
            }
        }
        else                                                                    // otherwise signalling mode (see 19.4.4)
        {
            if (bkn1_valid)
            {
                service_upper_mac(bkn1, SCH_HD);
            }

            if (bkn2_valid)
            {
                if (bnch_flag)
                {
                    service_upper_mac(bkn2, BNCH);
                }
                else
                {
                    service_upper_mac(bkn2, SCH_HD);
                }
            }
        }
    }
    else
    {
        // unkown burst type
        return;
    }
}

/**
 * @brief Process data in logical channel from lower mac
 *
 * See 23.2.2 MAC PDU mapping on logical channels
 *
 *    AACH             ACCESS-ASSIGN
 *    BSCH             SYNC
 *    BNCH on SCH/HD   SYSINFO
 *    SCH/F            MAC-DATA
 *    SCH/F or SCH/HD  MAC-RESOURCE
 *    SCH/F or SCH/HD  MAC-FRAG
 *    SCH/F or SCH/HD  MAC-END
 *    TCH_S
 *    TCH              MAC-TRAFFIC
 *
 *   AACH   = 0,
 *   BLCH   = 1,
 *   BNCH   = 2,
 *   BSCH   = 3,
 *   SCH_F  = 4,
 *   SCH_HD = 5,
 *   STCH   = 6,
 *   TCH_S  = 7,
 *   TCH    = 8,
 *   unkown = 9 *
 */

void tetra_dl::service_upper_mac(std::vector<uint8_t> data, mac_logical_channel_t mac_logical_channel)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - mac_channel = %s data = %s\n", "service_upper_mac", mac_logical_channel_name(mac_logical_channel).c_str(), vector_to_string(data, data.size()).c_str());
        fflush(stdout);
    }

    std::string txt = "?";
    uint8_t pdu_type;
    uint8_t sub_type;
    uint8_t broadcast_type;
    std::vector<uint8_t> tm_sdu;
    bool b_send_tm_sdu_to_llc = true;
    bool b_fragmented_packet  = false;

    mac_state.logical_channel = mac_logical_channel;

    switch (mac_logical_channel)
    {
    case AACH:
        mac_pdu_process_aach(data);                                             // ACCESS-ASSIGN see 21.4.7 - stop after processing
        txt = "  aach";
        break;

    case BSCH:                                                                  // SYNC PDU - stop after processing
        txt = "  bsch";
        tm_sdu = mac_pdu_process_sync(data);
        break;

    case TCH_S:                                                                 // (TMD) MAC-TRAFFIC PDU full slot
        printf("TCH_S       : TN/FN/MN = %2d/%2d/%2d    dl_usage_marker=%d\n", g_time.tn, g_time.fn, g_time.mn, mac_state.downlink_usage_marker);
        txt = "  tch_s";
        service_u_plane(data, TCH_S);
        break;

    case TCH:                                                                   // TCH half-slot TODO not taken into account for now
        printf("TCH         : TN/FN/MN = %2d/%2d/%2d    dl_usage_marker=%d\n", g_time.tn, g_time.fn, g_time.mn, mac_state.downlink_usage_marker);
        txt = "  tch";
        service_u_plane(data, TCH);
        break;

    case STCH:                                                                  // TODO stolen channel for signalling if MAC state in traffic mode -> user signalling, otherwise, signalling 19.2.4
    case BNCH:
    case SCH_F:
    case SCH_HD:
        // we are not in traffic mode
        pdu_type = get_value(data, 0, 2);

        switch (pdu_type)
        {
        case 0b00:                                                              // MAC PDU structure for downlink (TMA) MAC-RESSOURCE
            txt = "MAC-RESSOURCE";
            tm_sdu = mac_pdu_process_ressource(data, mac_logical_channel, &b_fragmented_packet);

            if (b_fragmented_packet)
            {
                // tm_sdu to be hold until MAC-END received
                b_send_tm_sdu_to_llc = false;
            }

            break;

        case 0b01:                                                              // MAC-FRAG or MAC-END (TMA)
            sub_type = get_value(data, 2, 1);
            if (sub_type == 0)                                                  // MAC-FRAG 21.4.3.2
            {
                txt = "MAC-FRAG";
                mac_pdu_process_mac_frag(data);                                 // no PDU returned // max 120 or 240 bits depending on channel
                b_send_tm_sdu_to_llc = false;
            }
            else                                                                // MAC-END 21.4.3.3
            {
                txt = "MAC-END";
                tm_sdu = mac_pdu_process_mac_end(data);
                b_send_tm_sdu_to_llc = true;
            }
            break;

        case 0b10:                                                              // MAC PDU structure for broadcast (TMB)  SYSINFO/ACCESS_DEFINE 21.4.4
            broadcast_type = get_value(data, 2, 2);
            switch (broadcast_type)
            {
            case 0b00:                                                          // SYSINFO see 21.4.4.1 / BNCH ???
                txt = "SYSINFO";
                tm_sdu = mac_pdu_process_sysinfo(data);                         // TM-SDU (MLE data)
                break;

            case 0b01:                                                          // ACCESS-DEFINE see 21.4.4.3, no sdu
                txt = "ACCESS-DEFINE";
                break;

            default:
                txt = "RESERVED";
            }
            break;

        case 0b11:                                                              // MAC-D-BLOCK (TMA)
            sub_type = get_value(data, 2, 1);

            if ((mac_logical_channel != STCH) && (mac_logical_channel != SCH_HD))
            {
                txt = "MAC-D-BLCK";                                             // 21.4.1 not sent on SCH/HD or STCH
                //tm_sdu = mac_pdu_process_d_block(data);
            }
            else
            {
                txt = "MAC-ERROR";
                printf("MAC error   : TN/FN/MN = %2d/%2d/%2d    supplementary block on channel %d\n", g_time.tn, g_time.fn, g_time.mn, mac_logical_channel);
            }
            break;

        default:
            txt = "pdu";
            break;
        }
        break;

    default:
        txt = "rev";
        break;
    }

    //printf("%-10s : TN/FN/MN = %2d/%2d/%2d    dl_usage_marker=%d\n", txt.c_str(), g_time.tn, g_time.fn, g_time.mn, mac_state.downlink_usage_marker);

    if ((tm_sdu.size() > 0) && b_send_tm_sdu_to_llc)
    {
        service_llc(tm_sdu, mac_logical_channel);                               // service LLC
    }

}

/**
 * @brief Decode length of MAC-RESSOURCE PDU - see 21.4.3.1 table 21.55
 *
 *        WARNING: length is in octet, not in bits
 *
 */

static uint32_t decode_length(uint32_t val)
{
    uint8_t  Y2  = 1;
    uint8_t  Z2  = 1;                                                           // for pi/4-DQPSK
    uint32_t ret = -1;

    if ((val == 0b000000) || (val == 0b111011) || val == (0b111100))
    {
        ret = -1;                                                               // reserved
    }
    else if (val <= 0b010010)
    {
        ret = val * Y2;
    }
    else if (val <= 0b111010)
    {
        ret = 18 * Y2 + (val - 18) * Z2;
    }
    else if (val == 0b111101)                                                   // QAM only
    {
        ret = -1;
    }
    else if (val == 0b111110)                                                   // second half slot stolen in STCH
    {
        ret = val;
    }
    else if (val == 0b111111)                                                   // start frag
    {
        ret = val;
    }

    return ret;
}

/**
 * @brief Process AACH - ACCESS-ASSIGN PDU - see 21.4.7, table 21.77
 *
 * Access field - 21.5.1
 * Control channel usage - 23.3.1.1
 *
 */

void tetra_dl::mac_pdu_process_aach(std::vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "mac_pdu_process_aach", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    uint8_t pos = 0;
    uint8_t header = get_value(pdu, pos, 2);
    pos += 2;
    uint8_t field1 = get_value(pdu, pos, 6);
    pos += 6;
    //uint8_t field2 = get_value(pdu, pos, 6);
    pos += 6;

    mac_state.downlink_usage_marker = 0;

    if (g_time.fn == 18)                                                        // frame 18 is reserved for control signalling - 23.3.1.3
    {
        mac_state.downlink_usage = COMMON_CONTROL;
    }
    else                                                                        // frame 1-17
    {
        if (header == 0b00)
        {
            mac_state.downlink_usage = COMMON_CONTROL;
        }
        else
        {
            switch (field1)
            {
            case 0b000000:
                mac_state.downlink_usage = UNALLOCATED;
                break;

            case 0b000001:
                mac_state.downlink_usage = ASSIGNED_CONTROL;
                break;

            case 0b000010:
                mac_state.downlink_usage = COMMON_CONTROL;
                break;

            case 0b000011:
                mac_state.downlink_usage = RESERVED;
                break;

            default:
                mac_state.downlink_usage = TRAFFIC;
                mac_state.downlink_usage_marker = field1;                       // note: 3 < field1 <= 63
                break;
            }
        }
    }

    //if(mac_state.downlink_usage == TRAFFIC)
    /*printf("AACH        : TN/FN/MN = %2d/%2d/%2d                        burst=%d  marker=%2u   field1 = %02u  field2 = %2u\n",
           g_time.tn,
           g_time.fn,
           g_time.mn,
           cur_burst_type,
           mac_state.downlink_usage_marker,
           field1,
           field2);*/
}

/**
 * @brief Remove fill bits - see 23.4.3.2
 *
 *
 */

std::vector<uint8_t> tetra_dl::mac_remove_fill_bits(const std::vector<uint8_t> pdu)
{
    std::vector<uint8_t> ret = pdu;

    if (g_remove_fill_bit_flag)
    {
        if (g_debug_level > 6)
        {
            printf(" ------- mac_remove_fill_bits BEFORE -- %u bits\n", (uint32_t)ret.size());
            print_vector(ret, ret.size());
        }

        if (ret[ret.size() - 1] == 1)
        {
            ret.resize(ret.size() - 1);                                         // 23.4.3.2 remove last 1
        }
        else
        {
            while (ret[ret.size() - 1] == 0)
            {
                ret.resize(ret.size() - 1);                                     // 23.4.3.2 remove all 0
            }
            ret.resize(ret.size() - 1);                                         // 23.4.3.2 then remove last 1
        }

        if (g_debug_level > 6)
        {
            printf(" ------- mac_remove_fill_bits AFTER --- %u bits\n", (uint32_t)ret.size());
            print_vector(ret, ret.size());
        }

    }

    return ret;
}

/**
 * @brief Process MAC-RESSOURCE and return TM-SDU (to LLC or MAC-FRAG) - see 21.4.3.1 table 329
 *
 * Maximum length (table 21.56):
 *    SCH/F   239 bits
 *    SCH/HD  95 bits
 *    STCH    95 bits
 *
 */

std::vector<uint8_t> tetra_dl::mac_pdu_process_ressource(std::vector<uint8_t> mac_pdu, mac_logical_channel_t mac_logical_channel, bool * b_fragmented_packet)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "mac_pdu_process_ressource", vector_to_string(mac_pdu, mac_pdu.size()).c_str());
        fflush(stdout);
    }

    std::vector<uint8_t> pdu = mac_pdu;

    *b_fragmented_packet = false;

    uint32_t pos = 2;                                                           // MAC pdu type

    uint8_t fill_bit_flag = get_value(pdu, pos, 1);                             // fill bit indication
    pos += 1;

    if (fill_bit_flag)
    {
        pdu = mac_remove_fill_bits(pdu);
    }

    pos += 1;                                                                   // position of grant
    pos += 2;                                                                   // encryption mode
    pos += 1;                                                                   // random access flag

    uint32_t length = get_value(pdu, pos, 6);                                   // length indication
    pos += 6;

    if (length == 0b111110)
    {
        second_slot_stolen_flag = 1;
    }
    else if (length == 0b111111)                                                // beginning of a fragmenting signalling message
    {
        *b_fragmented_packet = true;
        second_slot_stolen_flag = 0;
    }

    mac_address.address_type = get_value(pdu, pos, 3);
    pos += 3;

    if (mac_address.address_type == 0b000)                                      // NULL pdu, stop processing here
    {
        std::vector<uint8_t> null_pdu;
        return null_pdu;
    }
    else
    {
        switch (mac_address.address_type)                                       // TODO see EN 300 392-1 clause 7
        {
        case 0b001:                                                             // SSI
            mac_address.ssi = get_value(pdu, pos, 24);
            pos += 24;
            break;

        case 0b011:                                                             // USSI
            mac_address.ussi = get_value(pdu, pos, 24);
            pos += 24;
            break;

        case 0b100:                                                             // SMI
            mac_address.smi = get_value(pdu, pos, 24);
            pos += 24;
            break;

        case 0b010:                                                             // event label
            mac_address.event_label = get_value(pdu, pos, 10);
            pos += 10;
            break;

        case 0b101:                                                             // SSI + event label (event label assignment)
            mac_address.ssi = get_value(pdu, pos, 24);
            pos += 24;
            mac_address.event_label = get_value(pdu, pos, 10);
            pos += 10;
            break;

        case 0b110:                                                             // SSI + usage marker (usage marker assignment)
            mac_address.ssi = get_value(pdu, pos, 24);
            pos += 24;
            mac_address.usage_marker = get_value(pdu, pos, 6);
            pos += 6;
            break;

        case 0b111:                                                             // SMI + event label (event label assignment)
            mac_address.smi = get_value(pdu, pos, 24);
            pos += 24;
            mac_address.event_label = get_value(pdu, pos, 10);
            pos += 10;
            break;
        }

        if (get_value(pdu, pos, 1))                                             // power control flag
        {
            pos += 1 + 4;
        }
        else
        {
            pos += 1;
        }

        if (get_value(pdu, pos, 1))                                             // slot granting flag
        {
            pos += 1 + 8;
        }
        else
        {
            pos += 1;
        }

        uint8_t flag = get_value(pdu, pos, 1);
        pos += 1;
        if (flag)
        {
            uint8_t val;

            // 21.5.2 channel allocation elements table 341
            pos += 2;                                                           // channel allocation type
            pos += 4;                                                           // timeslot assigned
            uint8_t ul_dl = get_value(pdu, pos, 2);
            pos += 2;                                                           // up/downlink assigned
            pos += 1;                                                           // CLCH permission
            pos += 1;                                                           // cell change flag
            pos += 12;                                                          // carrier number
            flag = get_value(pdu, pos, 1);                                      // extended carrier numbering flag
            pos += 1;
            if (flag)
            {
                pos += 4;                                                       // frequency band
                pos += 2;                                                       // offset
                pos += 3;                                                       // duplex spacing
                pos += 1;                                                       // reverse operation
            }
            val = get_value(pdu, pos, 2);                                       // monitoring pattern
            pos += 2;
            if ((val == 0b00) && (g_time.fn == 18))                             // frame 18 conditional monitoring pattern
            {
                pos += 2;
            }

            if (ul_dl == 0)                                                     // augmented channel allocation
            {
                pos += 2;
                pos += 3;
                pos += 3;
                pos += 3;
                pos += 3;
                pos += 3;
                pos += 4;
                pos += 5;
                val = get_value(pdu, pos, 2);                                   // napping_sts
                pos += 2;
                if (val == 1)
                {
                    pos += 11;                                                  // 21.5.2c
                }
                pos += 4;

                flag = get_value(pdu, pos, 1);
                pos += 1;
                if (flag)
                {
                    pos += 16;
                }

                flag = get_value(pdu, pos, 1);
                pos += 1;
                if (flag)
                {
                    pos += 16;
                }

                pos += 1;
            }
        }
    }

    std::vector<uint8_t> sdu;

    // in case of NULL pdu, the length shall be 16 bits
    int32_t sdu_length = (int32_t)decode_length(length) * 8 - (int32_t)pos;

    if (sdu_length > 0)
    {
        // longest recommended size for TM_SDU 1106 bits = 133 bytes (with FCS) or 137 bytes (without FCS)
        // length includes MAC PDU header + TM_SDU length

        if (*b_fragmented_packet)
        {
            mac_defrag->start(mac_address, g_time);
            mac_defrag->append(vector_extract(pdu, pos, utils_substract(pdu.size(), pos)), mac_address); // length is the whole packet size - pos
        }
        else
        {
            sdu = vector_extract(pdu, pos, sdu_length);
        }
    }

    return sdu;
}

/**
 * @brief MAC-FRAG see 23.4.2.1 / 21.4.3.2 / 23.4.3 (defragmentation)
 *
 * Maximum length depends on channel (table 21.58):
 *   SCH/F  264 bits
 *   SCH/HD 120 bits
 *
 * Maximum consecutive slots N.203 >= 4 (Annex B.2)
 *
 */

void tetra_dl::mac_pdu_process_mac_frag(std::vector<uint8_t> mac_pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "mac_pdu_process_mac_frag", vector_to_string(mac_pdu, mac_pdu.size()).c_str());
        fflush(stdout);
    }

    std::vector<uint8_t> pdu = mac_pdu;

    uint32_t pos = 3;                                                           // MAC PDU type and subtype (MAC-FRAG)

    uint8_t fill_bit_flag = get_value(pdu, pos, 1);
    pos += 1;

    if (fill_bit_flag)
    {
        pdu = mac_remove_fill_bits(pdu);
    }

    std::vector<uint8_t> sdu = vector_extract(pdu, pos, utils_substract(pdu.size(), pos));

    mac_defrag->append(sdu, mac_address);
}

/**
 * @brief MAC-END 21.4.3.3 / 23.4.3 (defragmentation)
 *
 * Maximum length depends on channel (table 21.60):
 *   SCH/F  255 bits
 *   SCH/HD 111 bits
 *   STCH   111 bits
 *
 */

std::vector<uint8_t> tetra_dl::mac_pdu_process_mac_end(std::vector<uint8_t> mac_pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "mac_pdu_process_mac_end", vector_to_string(mac_pdu, mac_pdu.size()).c_str());
        fflush(stdout);
    }

    std::vector<uint8_t> pdu = mac_pdu;

    uint32_t pos = 3;                                                           // MAC PDU type and subtype (MAC-END)

    uint8_t fill_bit_flag = get_value(pdu, pos, 1);                             // fill bits
    pos += 1;

    if (fill_bit_flag)
    {
        pdu = mac_remove_fill_bits(pdu);
    }

    pos += 1;                                                                   // position of grant

    uint32_t val = get_value(pdu, pos, 6);                                      // length of MAC pdu
    pos += 6;

    if ((val < 0b000010) || (val > 0b100010))                                   // reserved
    {
        std::vector<uint8_t> null_pdu;
        return null_pdu;
    }

    //uint32_t length = decode_length(val);                                       // convert length in bytes (includes MAC PDU header + TM_SDU length)

    uint8_t flag = get_value(pdu, pos, 1);                                      // slot granting flag
    pos += 1;
    if (flag)
    {
        pos += 8;                                                               // slot granting element
    }

    flag = get_value(pdu, pos, 1);                                              // channel allocation flag
    pos += 1;
    if (flag)
    {
        // 21.5.2 channel allocation elements table 341
        pos += 2;                                                               // channel allocation type
        pos += 4;                                                               // timeslot assigned
        pos += 2;                                                               // up/downlink assigned
        pos += 1;                                                               // CLCH permission
        pos += 1;                                                               // cell change flag
        pos += 12;                                                              // carrier number
        flag = get_value(pdu, pos, 1);                                          // extended carrier numbering flag
        pos += 1;
        if (flag)
        {
            pos += 4;                                                           // frequency band
            pos += 2;                                                           // offset
            pos += 3;                                                           // duplex spacing
            pos += 1;                                                           // reverse operation
        }
        uint32_t val = get_value(pdu, pos, 2);                                  // monitoring pattern
        pos += 2;
        if ((val == 0b00) && (g_time.fn == 18))                                 // frame 18 conditional monitoring pattern
        {
            pos += 2;
        }
    }

    std::vector<uint8_t> sdu;

    mac_defrag->append(vector_extract(pdu, pos, utils_substract(pdu.size(), pos)), mac_address);
    sdu = mac_defrag->get_sdu();
    mac_defrag->stop();

    return sdu;
}

/**
 * @brief Process SYSINFO and return TM-SDU (MLE data) - see 21.4.4.1 table 333
 *
 */

std::vector<uint8_t> tetra_dl::mac_pdu_process_sysinfo(std::vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "mac_pdu_process_sysinfo", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    std::vector<uint8_t> sdu;

    static const std::size_t MIN_SIZE = 82;

    if (pdu.size() >= MIN_SIZE)
    {
        uint32_t pos = 4;
        uint16_t main_carrier = get_value(pdu, pos, 12);                        // main carrier frequency (1 / 25 kHz)
        pos += 12;

        uint8_t band_frequency = get_value(pdu, pos, 4);                        // frequency band (4 -> 400 MHz)
        pos += 4;

        uint8_t offset = get_value(pdu, pos, 2);                                // offset (0, 1, 2, 3)-> (0, +6.25, -6.25, +12.5 kHz)
        pos += 2;

        //uint8_t duplex_spacing = get_value(pdu, pos, 3);                            // duplex spacing;
        pos += 3;

        pos += 1;                                                               // reverse operation
        pos += 2;                                                               // number of common secondary control channels in use
        pos += 3;                                                               // MS_TXPWR_MAX_CELL
        pos += 4;                                                               // RXLEV_ACCESS_MIN
        pos += 4;                                                               // ACCESS_PARAMETER
        pos += 4;                                                               // RADIO_DOWNLINK_TIMEOUT

        uint8_t flag = get_value(pdu, pos, 1);
        pos += 1;                                                               // hyperframe / cipher key identifier flag
        if (flag)
        {
            pos += 16;                                                          // cyclic count of hyperframe
        }
        else
        {
            pos += 16;                                                          // common cipherkey identifier or static cipher key version number
        }

        pos += 2;                                                               // optional field flag
        pos += 20;                                                              // option value, always present

        // calculate cell frequencies

        const int32_t duplex[4] = {0, 6250, -6250, 12500};                      // 21.4.4.1

        g_cell_infos.downlink_frequency = (int32_t)band_frequency * 100000000 + (int32_t)main_carrier * 25000 + duplex[offset];
        g_cell_infos.uplink_frequency   = 0;                                    // TODO

        sdu = vector_extract(pdu, pos, 42);                                     // TM-SDU (MLE data) clause 18
    }
    else
    {
        report_add("invalid pdu size", (uint64_t)pdu.size());
        report_add("pdu minimum size", (uint64_t)MIN_SIZE);
    }

    return sdu;
}

/**
 * @brief Process MAC-D-BLCK - see 21.4.3.4 table 21.61
 *
 */

std::vector<uint8_t> tetra_dl::mac_pdu_process_d_block(std::vector<uint8_t> mac_pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "mac_pdu_process_d_block", vector_to_string(mac_pdu, mac_pdu.size()).c_str());
        fflush(stdout);
    }

    std::vector<uint8_t> pdu = mac_pdu;
    std::vector<uint8_t> sdu;

    static const std::size_t MIN_SIZE = 18;

    if (pdu.size() >= MIN_SIZE)
    {
        uint32_t pos = 3;

        uint8_t fill_bit_flag = get_value(pdu, pos, 1);                         // fill bits
        pos += 1;

        if (fill_bit_flag)
        {
            pdu = mac_remove_fill_bits(pdu);
        }

        pos += 2;                                                               // encryption mode
        mac_address.event_label = get_value(pdu, pos, 10);                      // address
        pos += 10;
        pos += 1;                                                               // immediate napping permission flag
        uint8_t flag = get_value(pdu, pos, 1);                                  // slot granting flag
        pos += 1;
        if (flag)                                                               // basic slot granting element
        {
            pos += 8;
        }

        sdu = vector_extract(pdu, pos, utils_substract(pdu.size(), pos));
    }
    else
    {
        report_add("invalid pdu size", (uint64_t)pdu.size());
        report_add("pdu minimum size", (uint64_t)MIN_SIZE);
    }

    return sdu;
}

/**
 * @brief Process SYNC - see 21.4.4.2 - Table 335
 *
 */

std::vector<uint8_t> tetra_dl::mac_pdu_process_sync(std::vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "mac_pdu_process_sync", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    std::vector<uint8_t> sdu;

    static const std::size_t MIN_SIZE = 60;

    if (pdu.size() >= MIN_SIZE)
    {
        uint32_t pos = 4;                                                       // system code
        g_cell_infos.color_code = get_value(pdu, pos, 6);
        pos += 6;
        g_time.tn = get_value(pdu, pos, 2) + 1;
        pos += 2;
        g_time.fn = get_value(pdu, pos, 5);
        pos += 5;
        g_time.mn = get_value(pdu, pos, 6);
        pos += 6;
        pos += 2;                                                               // sharing mode
        pos += 3;                                                               // reserved frames
        pos += 1;                                                               // U-plane DTX
        pos += 1;                                                               // frame 18 extension
        pos += 1;                                                               // reserved

        g_cell_infos.mcc = get_value(pdu, 31, 10);                              // should be done in MLE but we need it here to calculate scrambling code
        g_cell_infos.mnc = get_value(pdu, 41, 14);

        calculate_scrambling_code();
        g_cell_informations_acquired = true;

        report_start("MAC", "SYNC");
        report_send();

        if ((g_time.fn == 18) && ((g_time.mn + g_time.tn) % 4 == 3))
        {
            printf("BSCH        : TN/FN/MN = %2u/%2u/%2u  MAC-SYNC              ColorCode=%3d  MCC/MNC = %3u/ %3u  Freq= %10.6f MHz  burst=%u\n",
                   g_time.tn,
                   g_time.fn,
                   g_time.mn,
                   g_cell_infos.color_code,
                   g_cell_infos.mcc,
                   g_cell_infos.mnc,
                   g_cell_infos.downlink_frequency / 1.0e6,
                   cur_burst_type);
        }

        sdu = vector_extract(pdu, pos, 29);
    }
    else
    {
        report_add("invalid pdu size", (uint64_t)pdu.size());
        report_add("pdu minimum size", (uint64_t)MIN_SIZE);
    }

    return sdu;
}
