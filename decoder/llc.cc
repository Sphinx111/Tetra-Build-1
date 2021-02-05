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

/**
 * @brief LLC service entry point
 *
 * Process LLC PDU 21.2.1
 *
 */

void tetra_dl::service_llc(std::vector<uint8_t> pdu, mac_logical_channel_t mac_logical_channel)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - mac_channel = %s pdu = %s\n", "service_llc", mac_logical_channel_name(mac_logical_channel).c_str(), vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    if (mac_logical_channel == BSCH)                                           // TM-SDU is directly sent to MLE
    {
        service_mle(pdu, mac_logical_channel);
        return;
    }

    std::string txt = "";
    uint8_t advanced_link;
    uint8_t dfinal = -1;
    uint8_t ack_length = 0;

    std::vector<uint8_t> tl_sdu;

    uint32_t pos = 0;                                                           // current position in pdu stream
    uint8_t pdu_type = get_value(pdu, pos, 4);                                  // 21.2.1 table 21.1
    pos += 4;

    switch (pdu_type)
    {
    case 0b0000:                                                                // BL-ADATA
        txt = "BL-ADATA";
        pos += 1;                                                               // nr
        pos += 1;                                                               // ns
        tl_sdu = vector_extract(pdu, pos, utils_substract(pdu.size(), pos));
        break;

    case 0b0001:                                                                // BL-DATA
        txt = "BL-DATA";
        pos += 1;                                                               // ns
        tl_sdu = vector_extract(pdu, pos, utils_substract(pdu.size(), pos));
        break;

    case 0b0010:                                                                // BL-UDATA
        txt = "BL-UDATA";
        tl_sdu = vector_extract(pdu, pos, utils_substract(pdu.size(), pos));
        break;

    case 0b0011:                                                                // BL-ACK
        txt = "BL-ACK";
        pos += 1;                                                               // nr
        tl_sdu = vector_extract(pdu, pos, utils_substract(pdu.size(), pos));
        break;

    case 0b0100:                                                                // BL-ADATA + FCS
        txt = "BL-ADATA + FCS";
        pos += 1;                                                                 // nr
        pos += 1;                                                                 // ns
        tl_sdu = vector_extract(pdu, pos, utils_substract(pdu.size(), pos) - 32); // TODO removed FCS for now
        break;

    case 0b0101:                                                                // BL-DATA + FCS
        txt = "BL-DATA + FCS";
        tl_sdu = vector_extract(pdu, pos, utils_substract(pdu.size(), pos) - 32); // TODO removed FCS for now
        break;

    case 0b0110:                                                                // BL-UDATA + FCS
        txt = "BL-UDATA + FCS";
        tl_sdu = vector_extract(pdu, pos, utils_substract(pdu.size(), pos) - 32); // TODO removed FCS for now
        break;

    case 0b0111:                                                                // BL-ACK + FCS
        txt = "BL-ACK + FCS";
        pos += 1;                                                               // nr
        tl_sdu = vector_extract(pdu, pos, utils_substract(pdu.size(), pos) - 32); // TODO removed FCS for now
        break;

    case 0b1000:                                                                // AL-SETUP
        txt = "AL-SETUP";
        advanced_link = get_value(pdu, pos, 1);
        pos += 1;
        pos += 2;
        pos += 3;
        pos += 1;
        pos += 1;
        pos += 2;
        pos += 3;
        pos += 2;
        pos += 3;
        pos += 4;
        pos += 3;
        if (advanced_link == 0)
        {
            pos += 8;                                                           // ns
        }
        break;

    case 0b1001:                                                                // AL-DATA/AL-DATA-AR/AL-FINAL/AL-FINAL-AR
        dfinal = get_value(pdu, pos, 1);
        pos += 1;
        if (dfinal)
        {
            txt = "AL-FINAL/AL-FINAL-AR";
        }
        else
        {
            txt = "AL-DATA/AL-DATA-AR";
        }
        pos += 1;                                                               // ar
        pos += 3;                                                               // ns
        pos += 8;                                                               // ss
        tl_sdu = vector_extract(pdu, pos, utils_substract(pdu.size(), pos));
        break;

    case 0b1010:                                                                // AL-UDATA/AL-UFINAL
        dfinal = get_value(pdu, pos, 1); pos += 1;
        if (dfinal)
        {
            txt = "AL-UDATA";
        }
        else
        {
            txt = "AL-UFINAL";
        }
        pos += 8;                                                               // ns
        pos += 8;                                                               // ss
        tl_sdu = vector_extract(pdu, pos, utils_substract(pdu.size(), pos));
        break;

    case 0b1011:                                                                // AL-ACK/AL-UNR
        txt = "AL-ACK/AL-UNR";
        pos += 1;                                                               // flow control
        pos += 3;                                                               // nr - table 314 number of tl-sdu
        ack_length = get_value(pdu, pos, 6);
        pos += 6;
        if (ack_length >= 0b000001 && ack_length <= 0b111110)
        {
            pos += 8;                                                           // sr
        }
        else
        {

        }
        break;

    case 0b1100:                                                                // AL-RECONNECT
        txt = "AL-RECONNECT";
        break;

    case 0b1101:                                                                // supplementary LLC PDU (table 21.3)
        txt = "supplementary LLC PDU";
        break;

    case 0b1110:                                                                // layer-2 signalling PDU (table 21.2)
        txt = "layer 2 signalling PDU";
        break;

    case 0b1111:                                                                // AL-DISC
        txt = "AL-DISC";
        break;
    }

    if (g_debug_level > 1)
    {
        printf("service_llc : TN/FN/MN = %2d/%2d/%2d  %-20s\n", g_time.tn, g_time.fn, g_time.mn, txt.c_str());
    }

    if (tl_sdu.size() > 0)                                                      // service MLE
    {
        service_mle(tl_sdu, mac_logical_channel);
    }
}
