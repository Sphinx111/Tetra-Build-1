#include "tetra_dl.h"
#include "utils.h"

/**
 * @brief LIP protocol (stack built over SDS) - TS 100 392-18 - v1.7.3
 *
 *
 */

void tetra_dl::cmce_sds_service_location_information_protocol(vector<uint8_t> pdu)
{
    uint32_t pos = 0;                                                           // protocol ID from SDS has been removed since LIP is a service with SDU

    uint8_t pdu_type = get_value(pdu, pos, 2);                                  // see 6.2
    pos += 2;

    switch (pdu_type)                                                           // table 6.29
    {
    case 0b00:                                                                  // short location report
         report_add("sds-lip", "short location report");
         cmce_sds_lip_parse_short_location_report(pdu);
         break;

    case 0b01:                                                                  // location protocol PDU with extension
        break;

    case 0b10:                                                                  // reserved
    case 0b11:
        break;
    }
}

/**
 * @brief Decode LIP latitude - 6.3.30
 *
 *   11001010 00011001 01011001   Latitude    = 0x0CA1959   (-37.899131)
 *
 */

static double utils_decode_lip_latitude(uint32_t latitude)
{
    return utils_decode_integer_twos_complement(latitude, 24, 90.);
}

/**
 * @brief Decode LIP longitude - 6.3.50
 *
 * 0 11001110 01101100 10011000   Longitude   = 0x0CE6C98   (145.142012)
 *
 */

static double utils_decode_lip_longitude(uint32_t longitude)
{
    return utils_decode_integer_twos_complement(longitude, 25, 180.);
}

/**
 * @brief Short location report PDU - 6.2.1
 *
 */

void tetra_dl::cmce_sds_lip_parse_short_location_report(vector<uint8_t> pdu)
{
    uint32_t pos = 2;                                                           // PDU type

    pos += 2;                                                                   // time elapsed

    uint32_t longitude = get_value(pdu, pos, 25);
    pos += 25;
    report_add("longitude uint32", longitude);
    report_add("longitude", utils_decode_lip_longitude(longitude));

    uint32_t latitude = get_value(pdu, pos, 24);
    pos += 24;
    report_add("latitude uint32", longitude);
    report_add("latitude", utils_decode_lip_latitude(latitude));

    uint8_t position_error = get_value(pdu, pos, 3);
    pos += 3;

    uint8_t horizontal_velocity = get_value(pdu, pos, 7);
    pos += 7;

    uint8_t direction_of_travel = get_value(pdu, pos, 4);
    pos += 4;

    uint8_t type_of_additional_data = get_value(pdu, pos, 1);                   // 6.3.87 - Table 6.120
    pos += 1;

    uint8_t additional_data = get_value(pdu, pos, 8);
    pos += 8;

    if (type_of_additional_data == 0)                                           // reason for sending
    {
        report_add("reason for sending", additional_data);
    }
    else                                                                        // user-defined data
    {
        report_add("user-defined additional data", additional_data);
    }


}
