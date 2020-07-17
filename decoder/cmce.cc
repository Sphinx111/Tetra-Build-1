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
 * @brief CMCE service entry point - 14.7
 *
 */

void tetra_dl::service_cmce(vector<uint8_t> pdu, mac_logical_channel_t mac_logical_channel)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - mac_channel = %s pdu = %s\n", "service_cmce", mac_logical_channel_name(mac_logical_channel).c_str(), vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    uint8_t pdu_type;
    string txt = "";
    uint32_t cid = 0;

    bool b_complete_print_flag = true;

    uint32_t pos = 0;
    pdu_type = get_value(pdu, pos, 5);
    pos += 5;

    switch (pdu_type)
    {
    case 0b00000:
        txt = "D-ALERT";
        cmce_parse_d_alert(pdu);

        cid = get_value(pdu, pos, 14);
        pos += 14;
        break;

    case 0b00001:
        txt = "D-CALL-PROCEEDING";
        cmce_parse_d_call_proceeding(pdu);
        
        cid = get_value(pdu, pos, 14);
        pos += 14;
        break;

    case 0b00010:
        txt = "D-CONNECT";
        cmce_parse_d_connect(pdu);

        cid = get_value(pdu, pos, 14);
        pos += 14;
        break;

    case 0b00011:
        txt = "D-CONNECT ACK";
        cmce_parse_d_connect_ack(pdu);

        cid = get_value(pdu, pos, 14);
        pos += 14;
        break;

    case 0b00100:
        txt = "D-DISCONNECT";
        cmce_parse_d_disconnect(pdu);

        cid = get_value(pdu, pos, 14);
        pos += 14;
        break;

    case 0b00101:
        txt = "D-INFO";
        cmce_parse_d_info(pdu);

        cid = get_value(pdu, pos, 14);
        pos += 14;
        break;

    case 0b00110:
        txt = "D-RELEASE";
        cmce_parse_d_release(pdu);

        cid = get_value(pdu, pos, 14);
        pos += 14;
        break;

    case 0b00111:
        txt = "D-SETUP";
        cmce_parse_d_setup(pdu);

        cid = get_value(pdu, pos, 14);
        pos += 14;
        break;

    case 0b01000:
        txt = "D-STATUS";
        cmce_sds_parse_d_status(pdu);                                           // this pdu is handled by the SDS sub-entity see 14.7.1.10

        b_complete_print_flag = false;
        break;

    case 0b01001:
        txt = "D-TX CEASED";
        cmce_parse_d_tx_ceased(pdu);

        cid = get_value(pdu, pos, 14);
        pos += 14;
        break;

    case 0b01010:
        txt = "D-TX CONTINUE";
        cmce_parse_d_tx_continue(pdu);

        cid = get_value(pdu, pos, 14);
        pos += 14;
        break;

    case 0b01011:
        txt = "D-TX GRANTED";
        cmce_parse_d_tx_granted(pdu);

        cid = get_value(pdu, pos, 14);
        pos += 14;
        break;

    case 0b01100:
        txt = "D-TX WAIT";
        cmce_parse_d_tx_wait(pdu);

        cid = get_value(pdu, pos, 14);
        pos += 14;
        break;

    case 0b01101:
        txt = "D-TX INTERRUPT";
        cmce_parse_d_tx_interrupt(pdu);

        cid = get_value(pdu, pos, 14);
        pos += 14;
        break;

    case 0b01110:
        txt = "D-CALL RESTORE";
        cmce_parse_d_call_restore(pdu);

        cid = get_value(pdu, pos, 14);
        pos += 14;
        break;

    case 0b01111:
        txt = "D-SDS-DATA";
        cmce_sds_parse_d_sds_data(pdu);                                         // this pdu is handled by the SDS sub-entity see 14.7.1.11

        b_complete_print_flag = false;
        break;

    case 0b10000:
        txt = "D-FACILITY";                                                     // SS protocol
        break;

    case 0b11111:
        txt = "CMCE FUNCTION NOT SUPPORTED";
        break;
        
    default:
        txt = "reserved";
        break;
    }

    if (b_complete_print_flag)
    {
        printf("service_cmce: TN/FN/MN = %2d/%2d/%2d  %-20s  len=%3lu  cid=%u  ssi=%8u  usage_marker=%2u\n",
               g_time.tn,
               g_time.fn,
               g_time.mn,
               txt.c_str(),
               pdu.size(),
               cid,
               mac_address.ssi,
               mac_address.usage_marker);
    }
    else
    {
        printf("ser_cmce_sds: TN/FN/MN = %2d/%2d/%2d  %-20s  len=%3lu \n",
               g_time.tn,
               g_time.fn,
               g_time.mn,
               txt.c_str(),
               pdu.size());
    }
}

/**
 * @brief CMCE D-ALERT 14.7.1.1
 *
 */

void tetra_dl::cmce_parse_d_alert(vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "cmce_parse_d_alert", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    report_start("CMCE", "D-ALERT");

    uint32_t pos = 5;                                                           // pdu type

    report_add("call identifier", get_value(pdu, pos, 14));
    pos += 14;
    
    report_add("call timeout, setup phase", get_value(pdu, pos, 3));
    pos += 3;

    pos += 1;                                                                   // reserved

    report_add("simplex/duplex operation", get_value(pdu, pos, 1));
    pos += 1;

    report_add("call queued", get_value(pdu, pos, 1));
    pos += 1;

    // TODO type2 / 3 elements

    report_send();
}

/**
 * @brief CMCE D-CALL-PROCEEDING 14.7.1.2
 *
 */

void tetra_dl::cmce_parse_d_call_proceeding(vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "cmce_parse_d_call_proceeding", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    report_start("CMCE", "D-ALERT");

    uint32_t pos = 5;                                                           // pdu type

    report_add("call identifier", get_value(pdu, pos, 14));
    pos += 14;
    
    report_add("call timeout, setup phase", get_value(pdu, pos, 3));
    pos += 3;

    report_add("hook method selection", get_value(pdu, pos, 1));
    pos += 1;

    report_add("simplex/duplex selection", get_value(pdu, pos, 1));
    pos += 1;

    // TODO type2 / 3 elements

    report_send();
}

/**
 * @brief CMCE D-CONNECT 14.7.1.3
 *
 */

void tetra_dl::cmce_parse_d_call_restore(vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "cmce_parse_d_call_restore", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    report_start("CMCE", "D-CALL RESTORE");

    uint32_t pos = 5;                                                           // pdu type

    report_add("call identifier", get_value(pdu, pos, 14));
    pos += 14;

    report_add("transmission grant", get_value(pdu, pos, 2));
    pos += 2;

    report_add("transmission request permission", get_value(pdu, pos, 1));
    pos += 1;

    report_add("reset call time-out timer T310", get_value(pdu, pos, 1));
    pos += 1;

    uint8_t o_flag = get_value(pdu, pos, 1);                                    // option flag
    pos += 1;
    if (o_flag)                                                                 // there is type2 or type3/4 fields
    {
        uint8_t p_flag;                                                         // presence flag

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("new call identifier", get_value(pdu, pos, 14));
            pos += 14;
        }

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("call time-out", get_value(pdu, pos, 4));
            pos += 4;
        }

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("call status", get_value(pdu, pos, 3));
            pos += 3;
        }

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("modify", get_value(pdu, pos, 9));
            pos += 9;
        }

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("notification indicator", get_value(pdu, pos, 6));
            pos += 6;
        }

        // TODO handle type3/4 elements
    }

    report_send();
}

/**
 * @brief CMCE D-CONNECT 14.7.1.4
 *
 */

void tetra_dl::cmce_parse_d_connect(vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "cmce_parse_d_connect", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    report_start("CMCE", "D-CONNECT");

    uint32_t pos = 5;                                                           // pdu type

    report_add("call identifier", get_value(pdu, pos, 14));
    pos += 14;

    report_add("call timeout", get_value(pdu, pos, 4));
    pos += 4;

    report_add("hook method selection", get_value(pdu, pos, 1));
    pos += 1;

    report_add("simplex/duplex selection", get_value(pdu, pos, 1));
    pos += 1;

    report_add("transmission grant", get_value(pdu, pos, 2));
    pos += 2;

    report_add("transmission request permission", get_value(pdu, pos, 1));
    pos += 1;

    report_add("call ownership", get_value(pdu, pos, 1));
    pos += 1;

    uint8_t o_flag = get_value(pdu, pos, 1);                                    // option flag
    pos += 1;
    if (o_flag)                                                                 // there is type2 or type3/4 fields
    {
        uint8_t p_flag;                                                         // presence flag

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("call priority", get_value(pdu, pos, 4));
            pos += 4;
        }

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("basic service information", get_value(pdu, pos, 8));
            pos += 8;
        }

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("temporary address", get_value(pdu, pos, 24));
            pos += 24;
        }

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("notification indicator", get_value(pdu, pos, 6));
            pos += 6;
        }

        // TODO
        // uint8_t m_flag;                                                         // type 3/4 elements flag
        // m_flag = get_value(pdu, pos, 1);
        // pos += 1;

        // while (m_flag)                                                          // it there type3/4 fields
        // {
        //     // facility and proprietary elements
        //     report_add("type3 element id", json_object_new_int(get_value(pdu, pos, 4)));
        //     pos += 4;
        // }
    }

    report_send();
}

/**
 * @brief CMCE D-CONNECT ACK 14.7.1.5
 *
 */

void tetra_dl::cmce_parse_d_connect_ack(vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "cmce_parse_d_connect_ack", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    report_start("CMCE", "D-CONNECT ACK");

    uint32_t pos = 5;                                                           // pdu type

    report_add("call identifier", get_value(pdu, pos, 14));
    pos += 14;

    report_add("call timeout", get_value(pdu, pos, 4));
    pos += 4;

    report_add("transmission grant", get_value(pdu, pos, 2));
    pos += 2;

    report_add("transmission request permission", get_value(pdu, pos, 1));
    pos += 1;

    uint8_t o_flag = get_value(pdu, pos, 1);                                    // option flag
    pos += 1;

    if (o_flag)                                                                 // there is type2, type3 or type4 fields
    {
        uint8_t p_flag;                                                         // presence flag

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("notification indicator", get_value(pdu, pos, 6));
            pos += 6;
        }

        // TODO handle type3/4
    }

    report_send();
}

/**
 * @brief CMCE D-DISCONNECT 14.7.1.6
 *
 */

void tetra_dl::cmce_parse_d_disconnect(vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "cmce_parse_d_disconnect", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    report_start("CMCE", "D-DISCONNECT");

    uint32_t pos = 5;                                                           // pdu type

    report_add("call identifier", get_value(pdu, pos, 14));
    pos += 14;

    report_add("disconnect cause", get_value(pdu, pos, 1));
    pos += 1;

    uint8_t o_flag = get_value(pdu, pos, 1);                                    // option flag
    pos += 1;

    if (o_flag)                                                                 // there is type2, type3 or type4 fields
    {
        uint8_t p_flag;                                                         // presence flag

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("notification indicator", get_value(pdu, pos, 6));
            pos += 6;
        }

        // TODO handle type3/4
    }

    report_send();
}

/**
 * @brief CMCE D-INFO 14.7.1.8
 *
 */

void tetra_dl::cmce_parse_d_info(vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "cmce_parse_d_info", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    report_start("CMCE", "D-INFO");

    uint32_t pos = 5;                                                           // pdu type

    report_add("call identifier", get_value(pdu, pos, 14));
    pos += 14;

    report_add("reset call time-out timer (T310)", get_value(pdu, pos, 1));
    pos += 1;

    report_add("poll request", get_value(pdu, pos, 1));
    pos += 1;

    uint8_t o_flag = get_value(pdu, pos, 1);                                    // option flag
    pos += 1;

    if (o_flag)                                                                 // there is type2, type3 or type4 fields
    {
        uint8_t p_flag;                                                         // presence flag

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("new call identifier", get_value(pdu, pos, 14));
            pos += 14;
        }
        
        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("call time-out", get_value(pdu, pos, 4));
            pos += 4;
        }

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("call time-out setup phase (T301, T302)", get_value(pdu, pos, 3));
            pos += 3;
        }

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("call ownership", get_value(pdu, pos, 1));
            pos += 1;
        }

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("modify", get_value(pdu, pos, 9));
            pos += 9;
        }

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("call status", get_value(pdu, pos, 3));
            pos += 3;
        }

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("temporary address", get_value(pdu, pos, 24));
            pos += 24;
        }

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("notification indicator", get_value(pdu, pos, 6));
            pos += 6;
        }

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("poll response percentage", get_value(pdu, pos, 6));
            pos += 6;
        }

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("poll response number", get_value(pdu, pos, 6));
            pos += 6;
        }

        // TODO handle type3/4
    }

    report_send();
}

/**
 * @brief CMCE D-RELEASE 14.7.1.9
 *
 */

void tetra_dl::cmce_parse_d_release(vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "cmce_parse_d_release", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    report_start("CMCE", "D-RELEASE");

    uint32_t pos = 5;                                                           // pdu type

    report_add("call identifier", get_value(pdu, pos, 14));
    pos += 14;

    report_add("disconnect cause", get_value(pdu, pos, 5));
    pos += 5;

    uint8_t o_flag = get_value(pdu, pos, 1);                                    // option flag
    pos += 1;

    if (o_flag)                                                                 // there is type2, type3 or type4 fields
    {
        uint8_t p_flag;                                                         // presence flag

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("notification indicator", get_value(pdu, pos, 6));
            pos += 6;
        }

        // TODO handle type3/4
    }

    report_send();
}

/**
 * @brief CMCE D-SETUP 14.7.1.12
 *
 */

void tetra_dl::cmce_parse_d_setup(vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "cmce_parse_d_setup", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    report_start("CMCE", "D-SETUP");

    uint32_t pos = 5;                                                           // pdu type

    report_add("call identifier", get_value(pdu, pos, 14));
    pos += 14;

    report_add("call timeout", get_value(pdu, pos, 4));
    pos += 4;

    report_add("hook method selection", get_value(pdu, pos, 1));
    pos += 1;

    report_add("simplex/duplex selection", get_value(pdu, pos, 1));
    pos += 1;

    report_add("basic service information", get_value(pdu, pos, 8));
    pos += 8;

    report_add("transmission grant", get_value(pdu, pos, 2));
    pos += 2;

    report_add("transmission request permission", get_value(pdu, pos, 1));
    pos += 1;

    report_add("call priority", get_value(pdu, pos, 4));
    pos += 4;

    uint8_t o_flag = get_value(pdu, pos, 1);                                    // option flag
    pos += 1;

    if (o_flag)                                                                 // there is type2, type3 or type4 fields
    {
        uint8_t p_flag;                                                         // presence flag

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("notification indicator", get_value(pdu, pos, 6));
            pos += 6;
        }

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("temporary address", get_value(pdu, pos, 24));
            pos += 24;
        }

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)                                                             // calling party type identifier
        {
            uint8_t cpti = get_value(pdu, pos, 2);
            pos += 2;
            report_add("calling party type identifier", cpti);

            if (cpti == 0)                                                      // SNA ? not documented
            {
                report_add("calling party ssi", get_value(pdu, pos, 8));
                pos += 8;
            }
            else if (cpti == 1)
            {
                report_add("calling party ssi", get_value(pdu, pos, 24));
                pos += 24;
            }
            else if (cpti == 2)
            {
                report_add("calling party ssi", get_value(pdu, pos, 24));
                pos += 24;

                report_add("calling party ext", get_value(pdu, pos, 24));
                pos += 24;
            }
        }

        // TODO handle type 3/4
        // uint8_t m_flag;
        // m_flag = get_value(pdu, pos, 1);
        // pos += 1;
    }

    report_send();
}

/**
 * @brief CMCE D-TX CEASED 14.7.1.13
 *
 */

void tetra_dl::cmce_parse_d_tx_ceased(vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "cmce_parse_d_tx_ceased", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    report_start("CMCE", "D-TX CEASED");

    uint32_t pos = 5;                                                           // pdu type

    report_add("call identifier", get_value(pdu, pos, 14));
    pos += 14;

    report_add("transmission request permission", get_value(pdu, pos, 1));
    pos += 1;

    uint8_t o_flag = get_value(pdu, pos, 1);                                    // option flag
    pos += 1;

    if (o_flag)                                                                 // there is type2, type3 or type4 fields
    {
        uint8_t p_flag;                                                         // presence flag

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("notification indicator", get_value(pdu, pos, 6));
            pos += 6;
        }

        // TODO handle type3/4
    }

    report_send();
}

/**
 * @brief CMCE D-TX CONTINUE 14.7.1.14
 *
 */

void tetra_dl::cmce_parse_d_tx_continue(vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "cmce_parse_d_tx_continue", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    report_start("CMCE", "D-TX CONTINUE");

    uint32_t pos = 5;                                                           // pdu type

    report_add("call identifier", get_value(pdu, pos, 14));
    pos += 14;

    report_add("continue", get_value(pdu, pos, 1));
    pos += 1;

    report_add("transmission request permission", get_value(pdu, pos, 1));
    pos += 1;

    uint8_t o_flag = get_value(pdu, pos, 1);                                    // option flag
    pos += 1;

    if (o_flag)                                                                 // there is type2, type3 or type4 fields
    {
        uint8_t p_flag;                                                         // presence flag

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("notification indicator", get_value(pdu, pos, 6));
            pos += 6;
        }

        // TODO handle type3/4
    }

    report_send();
}

/**
 * @brief CMCE D-TX GRANTED 14.7.1.15
 *
 */

void tetra_dl::cmce_parse_d_tx_granted(vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "cmce_parse_d_tx_granted", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    report_start("CMCE", "D-TX GRANTED");

    uint32_t pos = 5;                                                           // pdu type

    report_add("call identifier", get_value(pdu, pos, 14));
    pos += 14;

    report_add("transmission grant", get_value(pdu, pos, 2));
    pos += 2;

    report_add("transmission request permission", get_value(pdu, pos, 1));
    pos += 1;

    report_add("encryption control", get_value(pdu, pos, 1));
    pos += 1;

    pos += 1;                                                                   // reserved and must be set to 0

    uint8_t o_flag = get_value(pdu, pos, 1);                                    // option flag
    pos += 1;

    if (o_flag)                                                                 // there is type2, type3 or type4 fields
    {
        uint8_t p_flag;                                                         // presence flag

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("notification indicator", get_value(pdu, pos, 6));
            pos += 6;
        }

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            uint8_t tpti = get_value(pdu, pos, 2);
            pos += 2;
            report_add("transmission party type identifier", tpti);

            if (tpti == 0)                                                      // SNA ? not documented
            {
                report_add("transmitting party ssi", get_value(pdu, pos, 8));
                pos += 8;
            }
            else if (tpti == 1)
            {
                report_add("transmitting party ssi", get_value(pdu, pos, 24));
                pos += 24;
            }
            else if (tpti == 2)
            {
                report_add("transmitting party ssi", get_value(pdu, pos, 24));
                pos += 24;

                report_add("transmitting party ext", get_value(pdu, pos, 24));
                pos += 24;
            }
        }

        // TODO handle type3/4
    }

    report_send();
}

/**
 * @brief CMCE D-TX INTERRUPT 14.7.1.16
 *
 */

void tetra_dl::cmce_parse_d_tx_interrupt(vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "cmce_parse_d_tx_interrupt", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    report_start("CMCE", "D-TX INTERRUPT");

    uint32_t pos = 5;                                                           // pdu type

    report_add("call identifier", get_value(pdu, pos, 14));
    pos += 14;

    report_add("transmission grant", get_value(pdu, pos, 2));
    pos += 2;

    report_add("transmission request permission", get_value(pdu, pos, 1));
    pos += 1;

    report_add("encryption control", get_value(pdu, pos, 1));
    pos += 1;

    pos += 1;                                                                   // reserved and must be set to 0

    uint8_t o_flag = get_value(pdu, pos, 1);                                    // option flag
    pos += 1;

    if (o_flag)                                                                 // there is type2, type3 or type4 fields
    {
        uint8_t p_flag;                                                         // presence flag

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("notification indicator", get_value(pdu, pos, 6));
            pos += 6;
        }

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            uint8_t tpti = get_value(pdu, pos, 2);
            pos += 2;
            report_add("transmission party type identifier", tpti);

            if (tpti == 0)                                                      // SNA ? not documented
            {
                report_add("transmitting party ssi", get_value(pdu, pos, 8));
                pos += 8;
            }
            else if (tpti == 1)
            {
                report_add("transmitting party ssi", get_value(pdu, pos, 24));
                pos += 24;
            }
            else if (tpti == 2)
            {
                report_add("transmitting party ssi", get_value(pdu, pos, 24));
                pos += 24;

                report_add("transmitting party ext", get_value(pdu, pos, 24));
                pos += 24;
            }
        }

        // TODO handle type3/4
    }

    report_send();
}

/**
 * @brief CMCE D-TX WAIT 14.7.1.17
 *
 */

void tetra_dl::cmce_parse_d_tx_wait(vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "cmce_parse_d_tx_wait", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    report_start("CMCE", "D-TX WAIT");

    uint32_t pos = 5;                                                           // pdu type

    report_add("call identifier", get_value(pdu, pos, 14));
    pos += 14;

    report_add("transmission request permission", get_value(pdu, pos, 1));
    pos += 1;

    uint8_t o_flag = get_value(pdu, pos, 1);                                    // option flag
    pos += 1;

    if (o_flag)                                                                 // there is type2, type3 or type4 fields
    {
        uint8_t p_flag;                                                         // presence flag

        p_flag = get_value(pdu, pos, 1);
        pos += 1;
        if (p_flag)
        {
            report_add("notification indicator", get_value(pdu, pos, 6));
            pos += 6;
        }

        // TODO handle type3/4
    }

    report_send();
}
