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
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - mac_channel = %s pdu = %s\n", "service_mle", mac_logical_channel_name(mac_logical_channel).c_str(), vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    //uint8_t pdu_type;
    string txt   = "";
    string infos = "";
    bool print_infos_flag = false;

    if (mac_logical_channel == BSCH)                                            // TM-SDU was already sent directly by MAC 18.4.2. Report infos and stop here
    {
        print_infos_flag = true;
        txt = "MLE";
        std::stringstream tt;
        tt << "D-MLE-SYNC (MCC=" << get_value(pdu, 0, 10)<< " MNC=" << get_value(pdu, 10, 14) << ")";
        infos = tt.str();
        return;                                                                 // TODO clean up
    }
    else if (mac_logical_channel == BNCH)                                       // TM-SDU was alrdeady sent directly by MAC 18.4.2. Report infos and stop here
    {
        print_infos_flag = true;
        txt = "MLE";
        std::stringstream tt;
        tt << "D-MLE-SYSINFO (LA=" << get_value(pdu, 0, 14) << " SS=" << get_value(pdu, 14, 16) << " BS=" << get_value(pdu, 30, 12) << ")";
        infos = tt.str();
        return;                                                                 // TODO clean up
    }
    else                                                                        // use discriminator - see 18.5.21
    {
        uint32_t pos = 0;
        uint8_t disc = get_value(pdu, pos, 3);
        pos += 3;

        switch (disc)
        {
        case 0b000:
            txt = "reserved";
            break;

        case 0b001:                                                             // transparent -> remove discriminator and send directly to MM
            txt = "MM";
            break;

        case 0b010:
            txt = "CMCE";                                                       // transparent -> remove discriminator and send directly to CMCE
            service_cmce(vector_extract(pdu, pos, utils_substract(pdu.size(), pos)), mac_logical_channel);
            break;

        case 0b011:
            txt = "reserved";
            break;

        case 0b100:
            txt = "SDNCP";                                                      // transparent -> remove discriminator and send directly to SDNCP
            break;

        case 0b101:                                                             // remove discriminator bits and send to MLE sub-system (for clarity only)
            txt = "MLE subsystem";
            service_mle_subsystem(vector_extract(pdu, pos, utils_substract(pdu.size(), pos)), mac_logical_channel);
            break;

        case 0b110:
        case 0b111:
            txt = "reserved";
            break;
        }
    }

    if (print_infos_flag || (g_debug_level > 1))
    {
        printf("service_mle : TN/FN/MN = %2d/%2d/%2d  %-20s  %-20s\n",
               g_time.tn,
               g_time.fn,
               g_time.mn,
               txt.c_str(),
               infos.c_str());
    }
}

/**
 * @brief Service MLE subsystem
 *
 */

void tetra_dl::service_mle_subsystem(vector<uint8_t> pdu, mac_logical_channel_t mac_logical_channel)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - mac_channel = %s pdu = %s\n", "service_mle_subsystem", mac_logical_channel_name(mac_logical_channel).c_str(), vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    string txt = "";

    uint32_t pos = 0;
    uint8_t pdu_type = get_value(pdu, pos, 3);
    pos += 3;

    switch (pdu_type)
    {
    case 0b000:
        txt = "D-NEW-CELL";
        break;

    case 0b001:
        txt = "D-PREPARE-FAIL";
        break;

    case 0b010:
        txt = "D-NWRK-BROADCAST";
        mle_process_d_nwrk_broadcast(pdu);
        break;

    case 0b011:
        txt = "D-NWRK-BROADCAST-EXTENSION";
        mle_process_d_nwrk_broadcast_extension(pdu);
        break;

    case 0b100:
        txt = "D-RESTORE-ACK";
        service_cmce(vector_extract(pdu, pos, utils_substract(pdu.size(), pos)), mac_logical_channel);
        break;

    case 0b101:
        txt = "D-RESTORE-FAIL";
        break;

    case 0b110:
        txt = "D-CHANNEL-RESPONSE";
        break;

    case 0b111:
        txt = "reserved";
        break;
    }

    printf("serv_mle_sub: TN/FN/MN = %2d/%2d/%2d  %-20s\n",                     // all MLE sub-system are printed
           g_time.tn,
           g_time.fn,
           g_time.mn,
           txt.c_str());
}

/**
 * @brief Process D-NWRK-BROADCAST 18.4.1.4.1
 *
 */

void tetra_dl::mle_process_d_nwrk_broadcast(vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "mle_process_d_nwrk_broadcast", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    report_start("MLE", "D-NWRK-BROADCAST");

    uint32_t pos = 3;                                                           // PDU type

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
            uint8_t neighbour_cells_count = get_value(pdu, pos, 3);
            pos += 3;
            report_add("number of neighbour cells", neighbour_cells_count);

            // for (uint8_t count = 0; count < neighbour_cells_count; count++)
            // {
            //     pos = mle_parse_neighbour_cell_information(pdu, pos, count);
            // }
        }
    }

    report_send();
}

/**
 * @brief Parse neighbour cell information 18.5.17 and return actual data length read
 *        to increase flux position. This function used by mle_process_d_nwrk_broadcast
 *
 */

uint32_t tetra_dl::mle_parse_neighbour_cell_information(vector<uint8_t> data, uint32_t pos_start, uint8_t idx)
{
    uint32_t pos = pos_start;

    string prefix = format_str("%s%u ", "cell", idx);

    report_add(prefix + "identifier", get_value(data, pos, 5));
    pos += 5;

    report_add(prefix + "reselection types supported", get_value(data, pos, 2));
    pos += 2;

    report_add(prefix + "neighbour cell synchronized", get_value(data, pos, 1));
    pos += 1;

    report_add(prefix + "service level", get_value(data, pos, 2));
    pos += 2;

    report_add(prefix + "main carrier number", get_value(data, pos, 12));
    pos += 12;

    uint8_t o_flag = get_value(data, pos, 1);                                   // option flag
    pos += 1;
    if (o_flag)                                                                 // there is type2 fields
    {
        uint8_t p_flag = get_value(data, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add(prefix + "main carrier number extension", get_value(data, pos, 10));
            pos += 10;
        }

        p_flag = get_value(data, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add(prefix + "MCC", get_value(data, pos, 10));
            pos += 10;
        }

        p_flag = get_value(data, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add(prefix + "MNC", get_value(data, pos, 14));
            pos += 14;
        }

        p_flag = get_value(data, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add(prefix + "LA", get_value(data, pos, 14));
            pos += 14;
        }

        p_flag = get_value(data, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add(prefix + "max. MS tx power", get_value(data, pos, 3));
            pos += 3;
        }

        p_flag = get_value(data, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add(prefix + "min. rx access level", get_value(data, pos, 4));
            pos += 4;
        }

        p_flag = get_value(data, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add(prefix + "subscriber class", get_value(data, pos, 16));
            pos += 16;
        }

        p_flag = get_value(data, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add(prefix + "BS service details", get_value(data, pos, 12));
            pos += 12;
        }

        p_flag = get_value(data, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add(prefix + "timeshare or security", get_value(data, pos, 5));
            pos += 5;
        }
        
        p_flag = get_value(data, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add(prefix + "TDMA frame offset", get_value(data, pos, 6));
            pos += 6;
        }
    }

    return pos;
}

/**
 * @brief Process D-NWRK-BROADCAST-EXTENSION 18.4.1.4.1a
 *
 */

void tetra_dl::mle_process_d_nwrk_broadcast_extension(vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "mle_process_d_nwrk_broadcast_extension", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    report_start("MLE", "D-NWRK-BROADCAST");

    uint32_t pos = 3;                                                           // PDU type

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
