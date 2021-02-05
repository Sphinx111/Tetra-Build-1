#include "tetra_dl.h"
#include "utils.h"
#include <math.h>

/**
 * @brief LIP protocol (stack built over SDS) - TS 100 392-18 - v1.7.3
 *
 *
 */

void tetra_dl::cmce_sds_service_location_information_protocol(std::vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "cmce_sds_service_location_information_protocol", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    uint32_t pos = 0;                                                           // protocol ID from SDS has been removed since LIP is a service with SDU

    uint8_t pdu_type = get_value(pdu, pos, 2);                                  // see 6.2
    pos += 2;

    switch (pdu_type)                                                           // table 6.29
    {
    case 0b00:                                                                  // short location report - 6.2.1
         report_add("sds-lip", "short location report");
         cmce_sds_lip_parse_short_location_report(pdu);
         break;

    case 0b01:                                                                  // PDU with extension - 6.3.62 table 6.92
        report_add("sds-lip", "extension pdu");
        cmce_sds_lip_parse_extended_message(pdu);
        break;

    case 0b10:                                                                  // reserved
    case 0b11:
        break;
    }
}

/**
 * @brief Parse LIP extension - see 6.3.62 table 6.92
 *
 */

void tetra_dl::cmce_sds_lip_parse_extended_message(std::vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "cmce_sds_lip_parse_extended_message", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    uint32_t pos = 2;                                                           // pdu type

    uint8_t extension = get_value(pdu, pos, 4);
    pos += 4;

    switch (extension)                                                          // table 6.92
    {
    case 0b0000:                                                                // reserved
    case 0b0010:
    case 0b1101:
    case 0b1110:
    case 0b1111:
        report_add("extension", "reserved");
        break;

    case 0b0001:
        report_add("extension", "immediate location report request");           // TODO 6.2.16
        break;

    case 0b0011:
        report_add("extension", "long location report");                        // TODO 6.2.2
        break;

    case 0b0100:
        report_add("extension", "location report ack");                         // TODO 6.2.3
        break;

    case 0b0101:
        report_add("extension", "basic location parameters request/response");  // TODO 6.2.4 - 6.2.5
        break;

    case 0b0110:
        report_add("extension", "add/modify trigger request/response");         // TODO 6.2.8 - 6.2.9
        break;

    case 0b0111:
        report_add("extension", "remove trigger request/response");             // TODO 6.2.10 - 6.2.11
        break;

    case 0b1000:
        report_add("extension", "report trigger request/response");             // TODO 6.2.12 - 6.2.13
        break;

    case 0b1001:
        report_add("extension", "report basic location parameters request/response"); // TODO 6.2.4 - 6.2.5
        break;

    case 0b1010:
        report_add("extension", "location reporting enable/disable request/response"); // TODO 6.2.14 - 6.2.15
        break;

    case 0b1011:
        report_add("extension", "location reporting temporary control request/response"); // TODO 6.2.17 - 6.2.18
        break;

    case 0b1100:
        report_add("extension", "backlog request/response");                    // TODO 6.2.19 - 6.2.20
        break;

    default:
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
 * @brief Decode LIP horizontal velocity - 6.3.17
 *
 */

static double utils_decode_lip_horizontal_velocity(uint8_t val)
{
    double res;

    if (val == 127)
    {
        res = -1.0;                                                             // unknown
    }
    else
    {
        const double C = 16.;
        const double x = 0.038;
        const double A = 13.;
        const double B = 0.;

        res = C * pow(1. + x, A - (double)val) + B;
    }

    return res;
}

/**
 * @brief Decode LIP direction on travel - 6.3.5
 *
 */

static std::string utils_decode_lip_direction_of_travel(uint8_t val)
{
    const std::string directions[] = {
        "0 N",   "22.5 NNE",  "45 NE",  "67.5 ENE",
        "90 E",  "112.5 ESE", "135 SE", "157.5 SSE",
        "180 S", "202.5 SSW", "225 SW", "247.5 WSW",
        "270 W", "292.5 WNW", "315 NW", "337.5 NNW"
    };

    return directions[val];
}

/**
 * @brief Decode LIP position error - 6.3.63
 *
 */

static std::string utils_decode_lip_position_error(uint8_t val)
{
    const std::string pos_error[] = {
        "< 2 m", "< 20 m", "< 200 m", "< 2 km", "< 20 km", "<= 200 km", "> 200 km", "unknown"
    };

    return pos_error[val];
};

/**
 * @brief Short location report PDU - 6.2.1
 *
 */

void tetra_dl::cmce_sds_lip_parse_short_location_report(std::vector<uint8_t> pdu)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - pdu = %s\n", "cmce_sds_lip_parse_short_location_report", vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    static const std::size_t MIN_SIZE = 68;

    if (pdu.size() >= MIN_SIZE)
    {
        uint32_t pos = 2;                                                       // PDU type

        pos += 2;                                                               // time elapsed

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
        report_add("position error", utils_decode_lip_position_error(position_error));

        uint8_t horizontal_velocity = get_value(pdu, pos, 7);
        pos += 7;
        report_add("horizontal_velocity uint8", horizontal_velocity);
        report_add("horizontal_velocity", utils_decode_lip_horizontal_velocity(horizontal_velocity));

        uint8_t direction_of_travel = get_value(pdu, pos, 4);
        pos += 4;
        report_add("direction of travel", utils_decode_lip_direction_of_travel(direction_of_travel));

        uint8_t type_of_additional_data = get_value(pdu, pos, 1);               // 6.3.87 - Table 6.120
        pos += 1;

        uint8_t additional_data = get_value(pdu, pos, 8);
        pos += 8;

        if (type_of_additional_data == 0)                                       // reason for sending
        {
            report_add("reason for sending", additional_data);
        }
        else                                                                    // user-defined data
        {
            report_add("user-defined additional data", additional_data);
        }
    }
    else
    {
        report_add("invalid pdu size", (uint64_t)pdu.size());
        report_add("pdu minimum size", (uint64_t)MIN_SIZE);
    }

}
