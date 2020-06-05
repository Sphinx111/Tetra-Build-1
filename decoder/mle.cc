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
 * @brief MLE service entry point
 *
 */

void tetra_dl::service_mle(vector<uint8_t> pdu, mac_logical_channel_t mac_logical_channel)
{
    vector<uint8_t> sdu;

    uint8_t pdu_type;
    string txt   = "";
    string infos = "";
    bool print_infos_flag = false;

    if (mac_logical_channel == BSCH)                                            // TM-SDU is directly sent by MAC 18.4.2
    {
        print_infos_flag = true;
        txt = "MLE";
        std::stringstream tt;
        tt << "D-MLE-SYNC (MCC=" << get_value(pdu, 0, 10)<< " MNC=" << get_value(pdu, 10, 14) << ")";
        infos = tt.str();
        return;                                                                 // TODO clean up
    }
    else if (mac_logical_channel == BNCH)                                       // TM-SDU is directly sent by MAC 18.4.2
    {
        print_infos_flag = true;
        txt = "MLE";
        std::stringstream tt;
        tt << "D-MLE-SYSINFO (LA=" << get_value(pdu, 0, 14) << " SS=" << get_value(pdu, 14, 16) << " BS=" << get_value(pdu, 30, 12) << ")";
        infos = tt.str();
        return;                                                                 // TODO clean up
    }
    else                                                                        // use discriminator
    {
        uint32_t pos = 0;
        uint8_t disc = get_value(pdu, pos, 3);
        pos += 3;

        switch (disc)
        {
        case 0b000:
            txt = "reserved";
            break;

        case 0b001:                                                             // transparent -> send directly to MM
            txt = "MM";
            break;

        case 0b010:
            txt = "CMCE";                                                       // transparent -> send directly to CMCE
            sdu = vector_extract(pdu, pos, pdu.size());
            service_cmce(sdu, mac_logical_channel);
            break;

        case 0b011:
            txt = "reserved";
            break;

        case 0b100:
            txt = "SDNCP";                                                      // transparent -> send directly to SDNCP
            break;

        case 0b101:
            print_infos_flag = true;                                            // allow report for MLE functions

            txt = "MLE";
            pdu_type = get_value(pdu, pos, 3);
            pos += 3;
            switch (pdu_type)
            {
            case 0b000:
                infos = "D-NEW-CELL";
                break;

            case 0b001:
                infos = "D-PREPARE-FAIL";
                break;

            case 0b010:
                infos = "D-NWRK-BROADCAST";
                mle_process_d_nwrk_broadcast(pdu);
                break;

            case 0b011:
                infos = "D-NWRK-BROADCAST-EXTENSION";
                mle_process_d_nwrk_broadcast_extension(pdu);
                break;

            case 0b100:
                infos = "D-RESTORE-ACK";
                sdu = vector_extract(pdu, pos, pdu.size());
                service_cmce(sdu, mac_logical_channel);
                break;

            case 0b101:
                infos = "D-RESTORE-FAIL";
                break;

            case 0b110:
                infos = "D-CHANNEL-RESPONSE";
                break;

            case 0b111:
                infos = "reserved";
                break;
            }
            break;

        case 0b110:
        case 0b111:
            txt = "reserved";
            break;
        }
    }

    if (print_infos_flag)
    {
        printf("service_mle : TN/FN/MN = %2d/%2d/%2d  %-20s\n",
               g_time.tn,
               g_time.fn,
               g_time.mn,
               infos.c_str());
    }
}

/**
 * @brief Process D-NWRK-BROADCAST 18.4.1.4.1
 *
 */

void tetra_dl::mle_process_d_nwrk_broadcast(vector<uint8_t> pdu)
{
    report_start("MLE", "D-NWRK-BROADCAST");

    uint32_t pos = 0;

    pos += 3;

    report_add("cell re-select parameter", get_value(pdu, pos, 16));
    pos += 16;

    report_add("cell service level", get_value(pdu, pos, 2));
    pos += 2;

    uint8_t o_flag = get_value(pdu, pos, 1);                                    // option flag
    pos += 1;
    if (o_flag)                                                                 // there is type2 or type3/4 fields
    {
        uint8_t p_flag;                                                         // presence flag

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("tetra network time", get_value(pdu, pos, 48));
            pos += 48;
        }

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("number of neighbour cells", get_value(pdu, pos, 3));
            pos += 3;
        }

        // TODO parse neighbour cells informations 18.5.17
    }

    report_send();
}

/**
 * @brief Process D-NWRK-BROADCAST-EXTENSION 18.4.1.4.1a
 *
 */

void tetra_dl::mle_process_d_nwrk_broadcast_extension(vector<uint8_t> pdu)
{
    report_start("MLE", "D-NWRK-BROADCAST");

    uint32_t pos = 0;

    pos += 3;

    uint8_t o_flag = get_value(pdu, pos, 1);                                    // option flag
    pos += 1;
    if (o_flag)                                                                 // there is type2 or type3/4 fields
    {
        uint8_t p_flag;                                                         // presence flag

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            uint8_t cnt = get_value(pdu, pos, 4);

            report_add("number of channel classes", cnt);
            pos += 4;

            // 18.5.5b Channel class
            // TODO parse channel class
        }

    }

    report_send();
}
