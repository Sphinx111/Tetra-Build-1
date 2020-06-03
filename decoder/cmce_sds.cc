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

/*
 * CMCE / SDS subsystem - see 14.5.5
 *
 */

/**
 * @brief CMCE D-SDS-DADA 14.7.1.10
 *
 * WARNING: this function generate two reports, the second one
 *          contains dump of user-defined type 4 message
 *
 */

void tetra_dl::cmce_sds_parse_d_sds_data(vector<uint8_t> pdu)
{
    report_start("CMCE", "D-SDS-DATA");

    uint32_t pos = 5;                                                           // pdu type

    bool b_valid = false;

    uint8_t cpti = get_value(pdu, pos, 2);
    pos += 2;
    report_add("calling party type identifier", cpti);

    if (cpti == 0)                                                              // not documented
    {
        report_add("calling party ssi", get_value(pdu, pos, 8));
        pos += 8;
        b_valid = true;
    }
    else if (cpti == 1)                                                         // SSI
    {
        report_add("calling party ssi", get_value(pdu, pos, 24));
        pos += 24;
        b_valid = true;
    }
    else if (cpti == 2)                                                         // SSI + EXT
    {
        report_add("calling party ssi", get_value(pdu, pos, 24));
        pos += 24;

        report_add("calling party ext", get_value(pdu, pos, 24));
        pos += 24;
        b_valid = true;
    }

    if (!b_valid)                                                               // can't process further, return
    {
        report_send();
        return;
    }

    uint8_t sdti = get_value(pdu, pos, 2);                                      // short data type identifier
    pos += 2;
    report_add("sds type identifier", sdti);

    vector<uint8_t> sdu;

    if (sdti == 0)                                                              // user-defined data 1
    {
        sdu = vector_extract(pdu, pos, 16);
        pos += 16;
        report_add("infos", sdu);
    }
    else if (sdti == 1)                                                         // user-defined data 2
    {
        sdu = vector_extract(pdu, pos, 32);
        pos += 32;
        report_add("infos", sdu);
    }
    else if (sdti == 2)                                                         // user-defined data 3
    {
        sdu = vector_extract(pdu, pos, 64);
        pos += 64;
        report_add("infos", sdu);
    }
    else if (sdti == 3)                                                         // length indicator + user-defined data 4
    {
        uint16_t len = get_value(pdu, pos, 11);                                 // length indicator
        pos += 11;

        sdu = vector_extract(pdu, pos, len);                                    // user-defined data 4
        pos += len;
        cmce_sds_parse_type4_data(sdu, len);                                    // parse type4 SDS message (message length is required to process user-defined type 4)
    }
    else
    {
        // invalid data
    }

    report_send();

    if (sdti == 3)                                                              // dump type 4 data sdu for analysis
    {
        report_start("CMCE", "D-SDS-DATA");
        report_add("hex", sdu);
        report_send();
    }

}

/**
 * @brief CMCE D-STATUS 14.7.1.11
 *
 */

void tetra_dl::cmce_sds_parse_d_status(vector<uint8_t> pdu)
{
    report_start("CMCE", "D-STATUS");

    uint32_t pos = 5;                                                           // pdu type

    uint8_t cpti = get_value(pdu, pos, 2);
    pos += 2;
    report_add("calling party type identifier", cpti);

    if (cpti == 1)                                                              // SSI
    {
        report_add("calling party ssi", get_value(pdu, pos, 24));
        pos += 24;
    }
    else if (cpti == 2)                                                         // SSI + EXT
    {
        report_add("calling party ssi", get_value(pdu, pos, 24));
        pos += 24;

        report_add("calling party ext", get_value(pdu, pos, 24));
        pos += 24;
    }

    report_add("pre-coded status", get_value(pdu, pos, 16));
    pos += 16;

    uint8_t o_flag = get_value(pdu, pos, 1);                                    // type 3 flag
    pos += 1;

    if (o_flag)                                                                 // there is type3 fields
    {
        uint8_t digits_count = get_value(pdu, pos, 8);
        pos += 8;

        string ext_number = "";
        for (int idx = 0; idx < digits_count; idx++)
        {
            ext_number += get_tetra_digit((uint8_t)get_value(pdu, pos, 4));
            pos += 4;
        }

        if ((digits_count % 2) != 0)                                            // check if we have a dummy digit
        {
            pos += 4;
        }
        report_add("external suscriber number", ext_number);

    }

    report_send();
}

/**
 * @brief SDS user-defined type 4 data - see 29.4, 29.5, Annex E, I, J
 *   Annex J - J.1 Protocol identifier information element
 *
 *   Protocol identifier 0b00000000 to 0b01111111 see clause 29.5 + Annex J.1 - Protocol specific definitions (shouldn't use SDS-TL) (text, location, OTAR...) see table 29.21
 *   Protocol identifier 0b10000000 to 0b11111111 see clause 29.4 + Annex J.1 - Other protocols with SDS-TL (NOTE some protocol may note use SDS-TL)
 *                       0b11000000 to 0b11111110
 *
 *   TODO check maximum length for user-defined data 4 is 2047 bits including protocol identifier 14.8.52
 */

void tetra_dl::cmce_sds_parse_type4_data(vector<uint8_t> pdu, const uint16_t len)
{
    uint32_t pos = 0;
    vector<uint8_t> sdu;
    
    uint8_t protocol_id = get_value(pdu, pos, 8);
    pos += 8;
    report_add("protocol id", protocol_id);                                     // Note that report has been opened and will be send cmce_sds_parse_d_sds_data function

    if (protocol_id <= 0b01111111)                                              // Non SDS-TL protocols - see Table 29.21
    {
        switch (protocol_id)
        {
        case 0b00000000:
            report_add("protocol info", "reserved");
            break;

        case 0b00000001:
            report_add("protocol info", "OTAK");
            // 29.5.1
            break;

        case 0b00000010:
            report_add("protocol info", "simple text messaging");               // 29.5.2
            cmce_sds_parse_simple_text_messaging(pdu, len);
            break;

        case 0b00000011:
            report_add("protocol info", "simple location system");              // 29.5.5
            cmce_sds_parse_simple_location_system(pdu, len);
            break;

        case 0b00000100:
            report_add("protocol info", "wireless datagram protocol");
            // 29.5.8
            break;

        case 0b00000101:
            report_add("protocol info", "wireless control message protocol");
            // 29.5.8
            break;

        case 0b00000110:
            report_add("protocol info", "M-DMO");                               // EN 300 396-10 [26]
            // 29.5.1
            break;

        case 0b00000111:
            report_add("protocol info", "pin authentification");
            // 29.5.1
            break;

        case 0b00001000:
            report_add("protocol info", "end-to-end encrypted message");
            break;

        case 0b00001001:
            report_add("protocol info", "simple intermediated text messaging");
            // 29.5.2
            break;

        case 0b00001010:
            report_add("protocol info", "location information protocol");       // 29.5.12 - TS 100 392-18 v1.7.2
            sdu = vector_extract(pdu, pos, len - pos);
            cmce_sds_service_location_information_protocol(sdu);                // LIP service
            break;

        case 0b00001011:
            report_add("protocol info", "net assist protocol");
            // 29.5.13
            break;

        case 0b00001100:
            report_add("protocol info", "concatenated sds message");
            // 29.5.14
            break;

        case 0b00001101:
            report_add("protocol info", "DOTAM");                               // TS 100 392-18-3 [48]
            // 29.5.1
            break;

        default:
            if (protocol_id <= 0b00111111)
            {
                report_add("protocol info", "reserved for future standard definition");
            }
            else if (protocol_id <= 0b00111110)
            {
                report_add("protocol info", "available for user application definition"); // Annex J
            }
            else                                                                // 0b0011111111
            {
                report_add("protocol info", "reserved for extension");          // TODO this value indicates that the next 8 bits is the protocol identifier and the 16 bits
                                                                                // replaces the 8 bits of the procol identifier in this PDU using this extension method
            }
            break;
        }
    }    // end of non SDS-TL protocols
    else                                                                        // SDS-TL (sub) protocols - see Table 29.21
    {
        // Annex J.1 - Table SDS-TL
        // protocol values in the scope of norm: 0b11000000 to 0b1111110

        uint8_t message_type = get_value(pdu, pos, 4);
        pos += 4;
        report_add("message type", message_type);

        switch (message_type)                                                   // 29.4.3.8 - Table 29.20
        {
        case 0b0000:                                                            // SDS-TRANSFER - 29.4.2.4 for user data type-4 informations
            report_add("sds-pdu", "SDS-TRANSFER");
            cmce_sds_parse_sub_d_transfer(pdu, len);
            break;

        case 0b0001:
            report_add("sds-pdu", "SDS-REPORT");                                // TODO don't process SDS message ?
            break;

        case 0b0010:
            report_add("sds-pdu", "SDS-ACK");                                   // TODO don't process SDS message ?
            break;

        default:                                                                // TODO don't process SDS message ?
            if (message_type <= 0b0111)
            {
                report_add("sds-pdu", "reserved for additional message types");
            }
            else                                                                // > 0b1000 29.4.3.8 - Table 29.20
            {
                report_add("protocol info", "defined by application");
            }

            break;
        }
    } // end of SDS-TL protocols
}

/**
 * @brief Parser sub protocol SDS-TRANFER
 *
 * {"service":"CMCE","pdu":"D-SDS-DATA","tn":1,"fn":2,"mn":43,"ssi":299906,"usage marker":0,"calling party type identifier":1,"calling party ssi":401101,"sds type identifier":3,"protocol id":130,"message type":0,"sds-pdu":"SDS-TRANSFER","message reference":227,"infos":"01 f0 80 00 c2 0c 12 07 29 05 99 05 82 01 0e 00 00 00"}
 * {"service":"CMCE","pdu":"D-SDS-DATA","tn":1,"fn":2,"mn":43,"ssi":299906,"usage marker":0,"hex":"82 00 e3 01 f0 80 00 c2 0c 12 07 29 05 99 05 82 01 0e 00 00 00"}
 *
 */

void tetra_dl::cmce_sds_parse_sub_d_transfer(vector<uint8_t> pdu, const uint16_t len)
{
    uint32_t pos = 0;
    uint8_t protocol_id = get_value(pdu, pos, 8);                               // protocol id
    pos += 4;                                                                   // message type (was SDS-TRANSFER)
    pos += 2;                                                                   // delivery report request
    pos += 1;                                                                   // service selection / short form report

    uint8_t service_forward_control = get_value(pdu, pos, 1);
    pos += 1;

    report_add("message reference", get_value(pdu, pos, 8));
    pos += 8;

    uint8_t digits_count = 0;
    string  ext_number   = "";

    if (service_forward_control)                                                // service forward control required
    {
        report_add("validity period", get_value(pdu, pos, 5));
        pos += 5;

        uint8_t forward_address_type = get_value(pdu, pos, 3);
        pos += 3;
        report_add("forward address type", forward_address_type);               // see 29.4.3.5

        switch (forward_address_type)
        {
        case  0b000:                                                            // SNA shouldn't be used (outside of scope of downlink receiver since it is reserved to MS -> SwMI direction)
            report_add("forward address ssi", get_value(pdu, pos, 8));
            pos += 8;
            break;

        case 0b001:                                                             // SSI
            report_add("forward address ssi", get_value(pdu, pos, 24));
            pos += 24;
            break;

        case 0b010:                                                             // TSI
            report_add("forward address ssi", get_value(pdu, pos, 24));
            pos += 24;

            report_add("forward address ext", get_value(pdu, pos, 24));
            pos += 24;
            break;

        case 0b011:                                                             // external subscriber number - CMCE type 3 block - 14.8.20
            digits_count = get_value(pdu, pos, 8);
            pos += 8;

            ext_number = "";
            for (int idx = 0; idx < digits_count; idx++)
            {
                ext_number += get_tetra_digit((uint8_t)get_value(pdu, pos, 4));
                pos += 4;
            }

            if ((digits_count % 2) != 0)                                        // check if we have a dummy digit
            {
                pos += 4;
            }
            report_add("forward address external number", ext_number);
            break;

        case 0b111:                                                             // no forward address present
            report_add("forward address", "none");
            break;

        default:                                                                // reserved
            break;
        }
    }

    vector<uint8_t> sdu = vector_extract(pdu, pos, len - pos);

    switch (protocol_id)                                                        // table 29.21
    {
    case 0b10000010:                                                            // 29.5.3
        cmce_sds_parse_text_messaging_with_sds_tl(sdu);
        report_add("protocol info", "text messaging (SDS-TL)");
        break;
        
    case 0b10000011:                                                            // 29.5.6
        cmce_sds_parse_location_system_with_sds_tl(sdu);
        report_add("protocol info", "location system (SDS-TL)");
        break;

    case 0b10000100:                                                            // Wireless Datagram Protocol WAP - 29.5.8
        report_add("protocol info", "WAP (SDS-TL)");
        break;
        
    case 0b10000101:                                                            // Wireless Control Message Protocol WCMP - 29.5.8
        report_add("protocol info", "WCMP (SDS-TL)");
        break;
        
    case 0b10000110:                                                            // Managed DMO M-DMO - 29.5.1
        report_add("protocol info", "M-DMO (SDS-TL)");
        break;
        
    case 0b10001000:                                                            // end-to-end encrypted message
        report_add("protocol info", "end-to-end encrypted message (SDS-TL)");
        break;
        
    case 0b10001001:                                                            // 29.5.3
        report_add("protocol info", "immediate text messaging (SDS-TL)");
        break;
        
    case 0b10001010:                                                            // UDH - 29.5.9
        report_add("protocol info", "message with user-data header");
        break;
        
    case 0b10001100:                                                            // 29.5.14
        report_add("protocol info", "concatenated sds message (SDS-TL)");
        break;
        
    default:
        break;
    }

    report_add("infos", sdu);                                                   // remaining part of pdu is sdu since we may be managed by SDS-TL
}

/**
 * @brief Parse simple text messaging - see 29.5.2
 *
 */

void tetra_dl::cmce_sds_parse_simple_text_messaging(vector<uint8_t> pdu, const uint16_t len)
{
    uint32_t pos = 8;                                                           // protocol id
    pos += 1;                                                                   // fill bit (should be 0) FIXME or timestamp 29.5.3.3 ?

    uint8_t text_coding_scheme = get_value(pdu, pos, 7);
    pos += 7;
    report_add("text coding scheme", text_coding_scheme);

    string txt = "";
    if (text_coding_scheme == 0b0000000)                                        // GSM 7-bit alphabet - see 29.5.4.3
    {
        txt = text_gsm_7_bit_decode(vector_extract(pdu, pos, len - pos), len - pos);
        report_add("infos", txt);
    }
    else if (text_coding_scheme <= 0b0011001)                                   // 8 bit alphabets
    {
        txt = text_generic_8_bit_decode(vector_extract(pdu, pos, len - pos), len - pos);
        report_add("infos", txt);
    }
    else
    {
        report_add("infos", vector_extract(pdu, pos, len - pos));               // hexadecimal report
    }
}

/**
 * @brief Parse text messaging with SDS-TL - see 29.5.3
 *
 */

void tetra_dl::cmce_sds_parse_text_messaging_with_sds_tl(vector<uint8_t> pdu)
{
    // Table 28.29 - 29.5.3.3
    uint16_t len = pdu.size();
    uint32_t pos = 0;
    
    uint8_t timestamp_flag = get_value(pdu, pos, 1);                            // timestamp flag
    pos += 1;
    
    uint8_t text_coding_scheme = get_value(pdu, pos, 7);
    pos += 7;
    report_add("text coding scheme", text_coding_scheme);

    if (timestamp_flag)
    {
        uint32_t timestamp = get_value(pdu, pos, 24);
        pos += 24;
        report_add("timestamp", timestamp);
    }
    
    string txt = "";
    if (text_coding_scheme == 0b0000000)                                        // GSM 7-bit alphabet - see 29.5.4.3
    {
        txt = text_gsm_7_bit_decode(vector_extract(pdu, pos, len - pos), len - pos);
        report_add("infos", txt);
    }
    else if (text_coding_scheme <= 0b0011001)                                   // 8 bit alphabets
    {
        txt = text_generic_8_bit_decode(vector_extract(pdu, pos, len - pos), len - pos);
        report_add("infos", txt);
    }
    else
    {
        report_add("infos", vector_extract(pdu, pos, len - pos));               // hexadecimal report
    }
}

/**
 * @brief Parse SDS simple location system - see 29.5.5
 *
 */

void tetra_dl::cmce_sds_parse_simple_location_system(vector<uint8_t> pdu, const uint16_t len)
{
    uint32_t pos = 8;                                                           // protocol id

    uint8_t location_system_coding = get_value(pdu, pos, 8);
    pos += 8;
    report_add("location coding system", location_system_coding);

    // remaining bits are len - 8 - 8 since len is size of pdu
    string txt = "";

    switch (location_system_coding)
    {
    case 0b00000000:                                                            // NMEA 0183 - see Annex L
        txt = location_nmea_decode(vector_extract(pdu, pos, len - pos), len - pos);
        report_add("infos", txt);
        break;

    case 0b00000001:                                                            // TODO RTCM RC-104 - see Annex L
        //txt = location_rtcm_decode(vector_extract(pdu, pos, len - pos), len - pos);
        report_add("infos", vector_extract(pdu, pos, len - pos));
        break;

    case 0b10000000:                                                            // Proprietary. Notes from SQ5BPF: some proprietary system seen in the wild in Spain, Itlay and France some speculate it's either from DAMM or SEPURA
        report_add("infos", vector_extract(pdu, pos, len - pos));
        break;

    default:
        report_add("infos", vector_extract(pdu, pos, len - pos));
        break;
    }
}

/**
 * @brief Parse SDS simple location system - see 29.5.5
 *
 */

void tetra_dl::cmce_sds_parse_location_system_with_sds_tl(vector<uint8_t> pdu)
{
    uint16_t len = pdu.size();
    uint32_t pos = 0;

    uint8_t location_system_coding = get_value(pdu, pos, 8);
    pos += 8;
    report_add("location coding system", location_system_coding);

    string txt = "";

    switch (location_system_coding)
    {
    case 0b00000000:                                                            // NMEA 0183 - see Annex L
        txt = location_nmea_decode(vector_extract(pdu, pos, len - pos), len - pos);
        report_add("infos", txt);
        break;

    case 0b00000001:                                                            // TODO RTCM RC-104 - see Annex L
        //txt = location_rtcm_decode(vector_extract(pdu, pos, len - pos), len - pos);
        report_add("infos", vector_extract(pdu, pos, len - pos));
        break;

    case 0b10000000:                                                            // Proprietary. Notes from SQ5BPF: some proprietary system seen in the wild in Spain, Itlay and France some speculate it's either from DAMM or SEPURA
        report_add("infos", vector_extract(pdu, pos, len - pos));
        break;

    default:
        report_add("infos", vector_extract(pdu, pos, len - pos));
        break;
    }
}

/**
 * @brief LIP protocol (stack built over SDS) - TS 100 392-18 - v1.7.3
 *
 * TODO move to another file
 *
 */

// void tetra_dl::cmce_sds_service_location_information_protocol(vector<uint8_t> pdu)
// {
//     uint16_t pos = 8;                                                           // protocol ID of Type 4 block

    
// }
